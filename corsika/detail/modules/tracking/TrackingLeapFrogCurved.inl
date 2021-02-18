/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/tracking/TrackingStraight.hpp> // for neutral particles
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/LeapFrogTrajectory.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/geometry/Intersections.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/QuarticSolver.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/modules/tracking/Intersect.hpp>

#include <type_traits>
#include <utility>

namespace corsika {

  namespace tracking_leapfrog_curved {

    template <typename TParticle>
    inline auto make_LeapFrogStep(TParticle const& particle, LengthType steplength) {
      if (particle.getMomentum().getNorm() == 0_GeV) {
        return std::make_tuple(particle.getPosition(), particle.getMomentum() / 1_GeV,
                               double(0));
      } // charge of the particle
      int const chargeNumber = particle.getChargeNumber();
      auto const* currentLogicalVolumeNode = particle.getNode();
      MagneticFieldVector const& magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(
              particle.getPosition());
      VelocityVector velocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;
      decltype(meter / (second * volt)) k =
          chargeNumber * constants::cSquared * 1_eV /
          (velocity.getNorm() * particle.getEnergy() * 1_V);
      DirectionVector direction = velocity.normalized();
      auto position = particle.getPosition(); // First Movement
      // assuming magnetic field does not change during movement
      position =
          position + direction * steplength / 2; // Change of direction by magnetic field
      direction =
          direction + direction.cross(magneticfield) * steplength * k; // Second Movement
      position = position + direction * steplength / 2;
      auto const steplength_true = steplength * (1 + direction.getNorm()) / 2;
      return std::make_tuple(position, direction.normalized(), steplength_true);
    }

    template <typename TParticle>
    inline auto Tracking::getTrack(TParticle const& particle) {
      VelocityVector const initialVelocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;

      auto const position = particle.getPosition();
      CORSIKA_LOG_DEBUG(
          "Tracking pid: {}"
          " , E = {} GeV",
          particle.getPID(), particle.getEnergy() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking pos: {}", position.getCoordinates());
      CORSIKA_LOG_DEBUG("Tracking   E: {} GeV", particle.getEnergy() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking   p: {} GeV",
                        particle.getMomentum().getComponents() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking   v: {} ", initialVelocity.getComponents());

      typedef
          typename std::remove_reference<decltype(*particle.getNode())>::type node_type;
      node_type& volumeNode = *particle.getNode();

      // for the event of magnetic fields and curved trajectories, we need to limit
      // maximum step-length since we need to follow curved
      // trajectories segment-wise -- at least if we don't employ concepts as "Helix
      // Trajectories" or similar
      MagneticFieldVector const& magneticfield =
          volumeNode.getModelProperties().getMagneticField(position);
      MagneticFluxType const magnitudeB = magneticfield.getNorm();
      int const chargeNumber = particle.getChargeNumber();
      bool const no_deflection = chargeNumber == 0 || magnitudeB == 0_T;

      if (no_deflection) { return getLinearTrajectory(particle); }

      HEPMomentumType const pAlongB_delta =
          (particle.getMomentum() -
           particle.getMomentum().getParallelProjectionOnto(magneticfield))
              .getNorm();

      if (pAlongB_delta == 0_GeV) {
        // particle travel along, parallel to magnetic field. Rg is
        // "0", but for purpose of step limit we return infinity here.
        CORSIKA_LOG_TRACE("pAlongB_delta is 0_GeV --> parallel");
        return getLinearTrajectory(particle);
      }

      LengthType const gyroradius =
          (pAlongB_delta * 1_V / (constants::c * abs(chargeNumber) * magnitudeB * 1_eV));

      double const maxRadians = 0.01;
      LengthType const steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
      TimeType const steplimit_time = steplimit / initialVelocity.getNorm();
      CORSIKA_LOG_DEBUG("gyroradius {}, steplimit: {} = {}", gyroradius, steplimit,
                        steplimit_time);

      // traverse the environment volume tree and find next
      // intersection
      auto [minTime, minNode] = nextIntersect(particle, steplimit_time);

      auto const k =
          chargeNumber * constants::cSquared * 1_eV / (particle.getEnergy() * 1_V);
      return std::make_tuple(
          LeapFrogTrajectory(position, initialVelocity, magneticfield, k,
                             minTime), // trajectory
          minNode);                    // next volume node
    }

    template <typename TParticle>
    inline Intersections Tracking::intersect(TParticle const& particle,
                                             Sphere const& sphere) {

      if (sphere.getRadius() == 1_km * std::numeric_limits<double>::infinity()) {
        return Intersections();
      }

      int const chargeNumber = particle.getChargeNumber();
      auto const& position = particle.getPosition();
      auto const* currentLogicalVolumeNode = particle.getNode();
      MagneticFieldVector const& magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(position);

      VelocityVector const velocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;
      DirectionVector const directionBefore =
          velocity.normalized(); // determine steplength to next volume

      auto const projectedDirection = directionBefore.cross(magneticfield);
      auto const projectedDirectionSqrNorm = projectedDirection.getSquaredNorm();
      bool const isParallel = (projectedDirectionSqrNorm == 0 * square(1_T));

      if (chargeNumber == 0 || magneticfield.getNorm() == 0_T || isParallel) {
        return tracking_line::Tracking::intersect<TParticle>(particle, sphere);
      }

      bool const numericallyInside = sphere.contains(particle.getPosition());
      CORSIKA_LOG_TRACE("numericallyInside={}", numericallyInside);

      auto const absVelocity = velocity.getNorm();
      auto const energy = particle.getEnergy();
      // this is: k = q/|p|
      auto const k =
          chargeNumber * constants::cSquared * 1_eV / (absVelocity * energy * 1_V);

      auto const direction_B_perp = directionBefore.cross(magneticfield);
      auto const denom = 4. / (direction_B_perp.getSquaredNorm() * k * k);
      Vector<length_d> const deltaPos = position - sphere.getCenter();
      double const b = (direction_B_perp.dot(deltaPos) * k + 1) * denom / (1_m * 1_m);
      double const c = directionBefore.dot(deltaPos) * 2 * denom / (1_m * 1_m * 1_m);
      LengthType const deltaPosLength = deltaPos.getNorm();
      double const d = (deltaPosLength + sphere.getRadius()) *
                       (deltaPosLength - sphere.getRadius()) * denom /
                       (1_m * 1_m * 1_m * 1_m);
      CORSIKA_LOG_TRACE("denom={}, b={}, c={}, d={}", denom, b, c, d);
      std::complex<double> const* solutions = quartic_solver::solve_quartic(0, b, c, d);
      LengthType d_enter, d_exit;
      int first = 0, first_entry = 0, first_exit = 0;
      for (int i = 0; i < 4; i++) {
        if (solutions[i].imag() == 0) {
          LengthType const dist = solutions[i].real() * 1_m;
          CORSIKA_LOG_TRACE("Solution (real) for current Volume: {} ", dist);
          if (numericallyInside) {
            // there must be an entry (negative) and exit (positive) solution
            if (dist < -0.0001_m) { // security margin to assure transfer to next
                                   // logical volume
              if (first_entry == 0) {
                d_enter = dist;
              } else {
                d_enter = std::max(d_enter, dist); // closest negative to zero (-1e-4) m
              }
              first_entry++;

            } else { // thus, dist > -0.0001_m

              if (first_exit == 0) {
                d_exit = dist;
              } else {
                d_exit = std::min(d_exit, dist); // closest positive to zero (-1e-4) m
              }
              first_exit++;
            }
            first = int(first_exit > 0) + int(first_entry > 0);

          } else { // thus, numericallyInside == false

            // both physical solutions (entry, exit) must be positive, and as small as
            // possible
            if (dist < -0.0001_m) { // need small numerical margin, to assure transport
              // into next logical volume
              continue;
            }
            if (first == 0) {
              d_enter = dist;
            } else {
              if (dist < d_enter) {
                d_exit = d_enter;
                d_enter = dist;
              } else {
                d_exit = dist;
              }
            }
            first++;
          }
        } // loop over solutions
      }
      delete[] solutions;

      if (first != 2) { // entry and exit points found
        CORSIKA_LOG_DEBUG("no intersection! count={}", first);
        return Intersections();
      }
      return Intersections(d_enter / absVelocity, d_exit / absVelocity);
    }

    template <typename TParticle>
    inline Intersections Tracking::intersect(TParticle const& particle,
                                             Plane const& plane) {

      int chargeNumber;
      if (is_nucleus(particle.getPID())) {
        chargeNumber = particle.getNuclearZ();
      } else {
        chargeNumber = get_charge_number(particle.getPID());
      }
      auto const* currentLogicalVolumeNode = particle.getNode();
      VelocityVector const velocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;
      auto const absVelocity = velocity.getNorm();
      DirectionVector const direction =
          velocity.normalized(); // determine steplength to next volume
      Point const position = particle.getPosition();

      auto const magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(position);

      if (chargeNumber != 0 && abs(plane.getNormal().dot(velocity.cross(magneticfield))) >
                                   1e-6_T * 1_m / 1_s) {

        auto const* currentLogicalVolumeNode = particle.getNode();
        auto const magneticfield =
            currentLogicalVolumeNode->getModelProperties().getMagneticField(position);
        auto const k =
            chargeNumber * (constants::c * 1_eV / 1_V) / particle.getMomentum().getNorm();

        auto const direction_B_perp = direction.cross(magneticfield);
        auto const denom = plane.getNormal().dot(direction_B_perp) * k;
        auto const sqrtArg =
            direction.dot(plane.getNormal()) * direction.dot(plane.getNormal()) -
            (plane.getNormal().dot(position - plane.getCenter()) * denom * 2);

        if (sqrtArg < 0) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);
        }
        double const sqrSqrtArg = sqrt(sqrtArg);
        auto const norm_projected =
            direction.dot(plane.getNormal()) / direction.getNorm();
        LengthType const MaxStepLength1 = (sqrSqrtArg - norm_projected) / denom;
        LengthType const MaxStepLength2 = (-sqrSqrtArg - norm_projected) / denom;

        CORSIKA_LOG_TRACE("MaxStepLength1={}, MaxStepLength2={}", MaxStepLength1,
                          MaxStepLength2);

        // check: both intersections in past
        if (MaxStepLength1 <= 0_m && MaxStepLength2 <= 0_m) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);

          // check: next intersection is MaxStepLength2
        } else if (MaxStepLength1 <= 0_m || MaxStepLength2 < MaxStepLength1) {
          CORSIKA_LOG_TRACE(" steplength to obs plane 2: {} ", MaxStepLength2);
          return Intersections(
              MaxStepLength2 *
              (direction + direction_B_perp * MaxStepLength2 * k / 2).getNorm() /
              absVelocity);

          // check: next intersections is MaxStepLength1
        } else if (MaxStepLength2 <= 0_m || MaxStepLength1 < MaxStepLength2) {
          CORSIKA_LOG_TRACE(" steplength to obs plane 2: {} ", MaxStepLength1);
          return Intersections(
              MaxStepLength1 *
              (direction + direction_B_perp * MaxStepLength1 * k / 2).getNorm() /
              absVelocity);
        }

        CORSIKA_LOG_WARN(
            "Particle wasn't tracked with curved trajectory -> straight (is this an "
            "ERROR?)");

      } // end if curved-tracking

      CORSIKA_LOG_TRACE("straight tracking with  chargeNumber={}, B={}", chargeNumber,
                        magneticfield);

      return tracking_line::Tracking::intersect(particle, plane);
    }

    template <typename TParticle, typename TBaseNodeType>
    inline Intersections Tracking::intersect(TParticle const& particle,
                                             TBaseNodeType const& volumeNode) {
      Sphere const* sphere = dynamic_cast<Sphere const*>(&volumeNode.getVolume());
      if (sphere) { return intersect(particle, *sphere); }
      throw std::runtime_error(
          "The Volume type provided is not supported in intersect(particle, node)");
    }

    template <typename TParticle>
    inline auto Tracking::getLinearTrajectory(TParticle& particle) {

      // perform simple linear tracking
      auto [straightTrajectory, minNode] = straightTracking_.getTrack(particle);

      // return as leap-frog trajectory
      return std::make_tuple(
          LeapFrogTrajectory(
              straightTrajectory.getLine().getStartPoint(),
              straightTrajectory.getLine().getVelocity(),
              MagneticFieldVector(particle.getPosition().getCoordinateSystem(), 0_T, 0_T,
                                  0_T),
              square(0_m) / (square(1_s) * 1_V),
              straightTrajectory.getDuration()), // trajectory
          minNode);                              // next volume node
    }

  } // namespace tracking_leapfrog_curved

} // namespace corsika
