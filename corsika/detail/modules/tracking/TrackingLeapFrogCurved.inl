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

      const double maxRadians = 0.01;
      const LengthType steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
      const TimeType steplimit_time = steplimit / initialVelocity.getNorm();
      CORSIKA_LOG_DEBUG("gyroradius {}, steplimit: {} = {}", gyroradius, steplimit,
                        steplimit_time);

      // traverse the environment volume tree and find next
      // intersection
      auto [minTime, minNode] = nextIntersect(particle, steplimit_time);

      const auto k =
          chargeNumber * constants::cSquared * 1_eV / (particle.getEnergy() * 1_V);
      return std::make_tuple(
          LeapFrogTrajectory(position, initialVelocity, magneticfield, k,
                             minTime), // trajectory
          minNode);                    // next volume node
    }

    template <typename TParticle, typename TMedium>
    inline Intersections Tracking::intersect(const TParticle& particle,
                                             const Sphere& sphere,
                                             const TMedium& medium) {

      if (sphere.getRadius() == 1_km * std::numeric_limits<double>::infinity()) {
        return Intersections();
      }

      int const chargeNumber = particle.getChargeNumber();
      auto const& position = particle.getPosition();
      MagneticFieldVector const& magneticfield = medium.getMagneticField(position);

      VelocityVector const velocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;
      DirectionVector const directionBefore =
          velocity.normalized(); // determine steplength to next volume

      auto const projectedDirection = directionBefore.cross(magneticfield);
      auto const projectedDirectionSqrNorm = projectedDirection.getSquaredNorm();
      bool const isParallel = (projectedDirectionSqrNorm == 0 * square(1_T));

      if (chargeNumber == 0 || magneticfield.getNorm() == 0_T || isParallel) {
        return tracking_line::Tracking::intersect<TParticle, TMedium>(particle, sphere,
                                                                      medium);
      }

      bool const numericallyInside = sphere.contains(particle.getPosition());

      const auto absVelocity = velocity.getNorm();
      auto energy = particle.getEnergy();
      auto k = chargeNumber * constants::cSquared * 1_eV / (absVelocity * energy * 1_V);

      auto const denom = (directionBefore.cross(magneticfield)).getSquaredNorm() * k * k;
      const double a =
          ((directionBefore.cross(magneticfield)).dot(position - sphere.getCenter()) * k +
           1) *
          4 / (1_m * 1_m * denom);
      const double b = directionBefore.dot(position - sphere.getCenter()) * 8 /
                       (denom * 1_m * 1_m * 1_m);
      const double c = ((position - sphere.getCenter()).getSquaredNorm() -
                        (sphere.getRadius() * sphere.getRadius())) *
                       4 / (denom * 1_m * 1_m * 1_m * 1_m);
      CORSIKA_LOG_TRACE("denom={}, a={}, b={}, c={}", denom, a, b, c);
      std::complex<double>* solutions = quartic_solver::solve_quartic(0, a, b, c);
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

            } else { // thus, dist >= -0.0001_m

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

    template <typename TParticle, typename TMedium>
    Intersections Tracking::intersect(TParticle const& particle, Plane const& plane,
                                      TMedium const& medium) {

      int chargeNumber;
      if (is_nucleus(particle.getPID())) {
        chargeNumber = particle.getNuclearZ();
      } else {
        chargeNumber = get_charge_number(particle.getPID());
      }
      auto const* currentLogicalVolumeNode = particle.getNode();
      auto magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(
              particle.getPosition());
      VelocityVector const velocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;
      auto const absVelocity = velocity.getNorm();
      DirectionVector const direction =
          velocity.normalized(); // determine steplength to next volume
      Point const position = particle.getPosition();

      if (chargeNumber != 0 &&
          abs(plane.getNormal().dot(velocity.cross(magneticfield))) * 1_s / 1_m / 1_T >
              1e-6) {

        auto const* currentLogicalVolumeNode = particle.getNode();
        auto magneticfield =
            currentLogicalVolumeNode->getModelProperties().getMagneticField(
                particle.getPosition());
        auto k =
            chargeNumber * constants::c * 1_eV / (particle.getMomentum().getNorm() * 1_V);

        if (direction.dot(plane.getNormal()) * direction.dot(plane.getNormal()) -
                (plane.getNormal().dot(position - plane.getCenter()) *
                 plane.getNormal().dot(direction.cross(magneticfield)) * 2 * k) <
            0) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);
        }

        LengthType const MaxStepLength1 =
            (sqrt(direction.dot(plane.getNormal()) * direction.dot(plane.getNormal()) -
                  (plane.getNormal().dot(position - plane.getCenter()) *
                   plane.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
             direction.dot(plane.getNormal()) / direction.getNorm()) /
            (plane.getNormal().dot(direction.cross(magneticfield)) * k);

        LengthType const MaxStepLength2 =
            (-sqrt(direction.dot(plane.getNormal()) * direction.dot(plane.getNormal()) -
                   (plane.getNormal().dot(position - plane.getCenter()) *
                    plane.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
             direction.dot(plane.getNormal()) / direction.getNorm()) /
            (plane.getNormal().dot(direction.cross(magneticfield)) * k);

        if (MaxStepLength1 <= 0_m && MaxStepLength2 <= 0_m) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);
        } else if (MaxStepLength1 <= 0_m || MaxStepLength2 < MaxStepLength1) {
          CORSIKA_LOG_TRACE(" steplength to obs plane 2: {} ", MaxStepLength2);
          return Intersections(
              MaxStepLength2 *
              (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                  .getNorm() /
              absVelocity);
        } else if (MaxStepLength2 <= 0_m || MaxStepLength1 < MaxStepLength2) {
          CORSIKA_LOG_TRACE(" steplength to obs plane 2: {} ", MaxStepLength1);
          return Intersections(
              MaxStepLength1 *
              (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                  .getNorm() /
              absVelocity);
        }

        CORSIKA_LOG_WARN("Particle wasn't tracked with curved trajectory -> straight");

      } // end if curved-tracking

      return tracking_line::Tracking::intersect(particle, plane, medium);
    }

    template <typename TParticle, typename TBaseNodeType>
    inline Intersections Tracking::intersect(const TParticle& particle,
                                             const TBaseNodeType& volumeNode) {
      const Sphere* sphere = dynamic_cast<const Sphere*>(&volumeNode.getVolume());
      if (sphere) {
        return intersect(particle, *sphere, volumeNode.getModelProperties());
      }
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
