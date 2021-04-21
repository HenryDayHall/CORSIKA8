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
      ElectricChargeType const charge = particle.getCharge();
      auto const* currentLogicalVolumeNode = particle.getNode();
      MagneticFieldVector const& magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(
              particle.getPosition());
      VelocityVector velocity = particle.getVelocity();
      decltype(meter / (second * volt)) k =
          charge * constants::cSquared * 1_eV /
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
      VelocityVector const initialVelocity = particle.getVelocity();

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
      node_type const& volumeNode = *particle.getNode();

      // for the event of magnetic fields and curved trajectories, we need to limit
      // maximum step-length since we need to follow curved
      // trajectories segment-wise -- at least if we don't employ concepts as "Helix
      // Trajectories" or similar
      MagneticFieldVector const& magneticfield =
          volumeNode.getModelProperties().getMagneticField(position);
      MagneticFluxType const magnitudeB = magneticfield.getNorm();
      ElectricChargeType const charge = particle.getCharge();
      bool const no_deflection = (charge == 0 * constants::e) || magnitudeB == 0_T;

      if (no_deflection) { return getLinearTrajectory(particle); }

      HEPMomentumType const p_perp =
          (particle.getMomentum() -
           particle.getMomentum().getParallelProjectionOnto(magneticfield))
              .getNorm();

      if (p_perp < 1_eV) {
        // particle travel along, parallel to magnetic field. Rg is
        // "0", but for purpose of step limit we return infinity here.
        CORSIKA_LOG_TRACE("p_perp is 0_GeV --> parallel");
        return getLinearTrajectory(particle);
      }

      LengthType const gyroradius = (convert_HEP_to_SI<MassType::dimension_type>(p_perp) *
                                     constants::c / (abs(charge) * magnitudeB));

      double const maxRadians = 0.01;
      LengthType const steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
      TimeType const steplimit_time = steplimit / initialVelocity.getNorm();
      CORSIKA_LOG_DEBUG("gyroradius {}, steplimit: {} = {}", gyroradius, steplimit,
                        steplimit_time);

      // traverse the environment volume tree and find next
      // intersection
      auto [minTime, minNode] = nextIntersect(particle, steplimit_time);

      auto const p_norm =
          constants::c * convert_HEP_to_SI<MassType::dimension_type>(
                             particle.getMomentum().getNorm()); // kg *m /s
      // k = q/|p|
      decltype(1 / (tesla * second)) const k =
          charge / p_norm *
          initialVelocity.getNorm(); // since we use steps in time and not length
      // units: C * s / m / kg * m/s = 1 / (T*m) * m/s = 1/(T s)

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

      ElectricChargeType const charge = particle.getCharge();
      auto const& position = particle.getPosition();
      auto const* currentLogicalVolumeNode = particle.getNode();
      MagneticFieldVector const& magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(position);

      VelocityVector const velocity = particle.getVelocity();
      DirectionVector const directionBefore = velocity.normalized();

      auto const projectedDirection = directionBefore.cross(magneticfield);
      auto const projectedDirectionSqrNorm = projectedDirection.getSquaredNorm();
      bool const isParallel = (projectedDirectionSqrNorm == 0 * square(1_T));

      if ((charge == 0 * constants::e) || magneticfield.getNorm() == 0_T || isParallel) {
        return tracking_line::Tracking::intersect<TParticle>(particle, sphere);
      }

      bool const numericallyInside = sphere.contains(particle.getPosition());
      CORSIKA_LOG_TRACE("numericallyInside={}", numericallyInside);

      SpeedType const absVelocity = velocity.getNorm();
      auto const p_norm =
          constants::c * convert_HEP_to_SI<MassType::dimension_type>(
                             particle.getMomentum().getNorm()); // km * m /s
      // this is: k = q/|p|
      auto const k = charge / p_norm;

      MagneticFieldVector const direction_x_B = directionBefore.cross(magneticfield);
      auto const denom = 4. / (direction_x_B.getSquaredNorm() * k * k);
      Vector<length_d> const deltaPos = position - sphere.getCenter();
      double const b = (direction_x_B.dot(deltaPos) * k + 1) * denom / (1_m * 1_m);
      double const c = directionBefore.dot(deltaPos) * 2 * denom / (1_m * 1_m * 1_m);
      LengthType const deltaPosLength = deltaPos.getNorm();
      double const d = (deltaPosLength + sphere.getRadius()) *
                       (deltaPosLength - sphere.getRadius()) * denom /
                       (1_m * 1_m * 1_m * 1_m);
      CORSIKA_LOG_TRACE("denom={}, b={}, c={}, d={}", denom, b, c, d);
      std::vector<double> solutions = andre::solve_quartic_real(1, 0, b, c, d);
      LengthType d_enter, d_exit;
      int first = 0, first_entry = 0, first_exit = 0;
      for (auto solution : solutions) {
        LengthType const dist = solution * 1_m;
        CORSIKA_LOG_TRACE("Solution (real) for current Volume: {} ", dist);
        if (numericallyInside) {
          // there must be an entry (negative) and exit (positive) solution
          if (dist < 0.0001_m) { // security margin to assure
                                 // transfer to next logical volume
                                 // (even if dist suggest marginal
                                 // entry already, which we
                                 // classify as numerical artifact)
            if (first_entry == 0) {
              d_enter = dist;
            } else {
              d_enter = std::max(d_enter, dist); // closest negative to zero >1e-4 m
            }
            first_entry++;

          } else { // thus, dist > +0.0001_m

            if (first_exit == 0) {
              d_exit = dist;
            } else {
              d_exit = std::min(d_exit, dist); // closest positive to zero >1e-4 m
            }
            first_exit++;
          }
          first = int(first_exit > 0) + int(first_entry > 0);

        } else { // thus, numericallyInside == false

          // both physical solutions (entry, exit) must be positive, and as small as
          // possible
          if (dist < -0.0001_m) { // need small numerical margin, to
                                  // assure transport. We consider
                                  // begin marginally already inside
                                  // next volume (besides
                                  // numericallyInside=false) as numerical glitch.
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

      if (first == 0) { // entry and exit points found
        CORSIKA_LOG_DEBUG(
            "no intersections found: count={}, first_entry={}, first_exit={}", first,
            first_entry, first_exit);
        return Intersections();
      }
      return Intersections(d_enter / absVelocity, d_exit / absVelocity);
    }

    template <typename TParticle>
    inline Intersections Tracking::intersect(TParticle const& particle,
                                             Plane const& plane) {

      CORSIKA_LOG_TRACE("intersection particle with plane");

      ElectricChargeType const charge = particle.getCharge();

      if (charge != 0 * constants::e) {

        auto const* currentLogicalVolumeNode = particle.getNode();
        VelocityVector const velocity = particle.getVelocity();
        auto const absVelocity = velocity.getNorm();
        DirectionVector const direction = velocity.normalized();
        Point const position = particle.getPosition();

        auto const magneticfield =
            currentLogicalVolumeNode->getModelProperties().getMagneticField(position);

        // solve:     denom x^2 + p x + q =0    for     x = delta-l

        auto const direction_x_B = direction.cross(magneticfield);
        double const denom = charge *
                             plane.getNormal().dot(direction_x_B) // unit: C*T = kg/s
                             / 1_kg * 1_s;

        CORSIKA_LOG_TRACE("denom={}", denom);

        auto const p_norm =
            constants::c * convert_HEP_to_SI<MassType::dimension_type>(
                               particle.getMomentum().getNorm()); // unit: kg * m/s

        double const p = (2 * p_norm * direction.dot(plane.getNormal())) // unit: kg*m/s
                         / (1_m * 1_kg) * 1_s;
        double const q =
            (2 * p_norm *
             plane.getNormal().dot(position - plane.getCenter())) // unit: kg*m/s *m
            / (1_m * 1_m * 1_kg) * 1_s;

        std::vector<double> deltaLs = solve_quadratic_real(denom, p, q);

        CORSIKA_LOG_TRACE("deltaLs=[{}]", fmt::join(deltaLs, ", "));

        if (deltaLs.size() == 0) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);
        }

        // select smallest but positive solution
        bool first = true;
        LengthType maxStepLength = 0_m;
        for (auto& deltaL : deltaLs) {
          if (deltaL < 0) continue;
          if (first) {
            first = false;
            maxStepLength = deltaL * meter;
          } else if (maxStepLength > deltaL * meter) {
            maxStepLength = deltaL * meter;
          }
        }

        // check: both intersections in past, or no valid intersection
        if (first) {
          return Intersections(std::numeric_limits<double>::infinity() * 1_s);
        }

        CORSIKA_LOG_TRACE("maxStepLength={}", maxStepLength);

        // with final length correction
        auto const corr =
            (direction + direction_x_B * maxStepLength * charge / (p_norm * 2))
                .getNorm() /
            absVelocity; // unit: s/m

        return Intersections(maxStepLength * corr); // unit: s

      } // no charge

      CORSIKA_LOG_TRACE("(plane) straight tracking with  charge={}, B={}", charge,
                        particle.getNode()->getModelProperties().getMagneticField(
                            particle.getPosition()));

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
              0 * square(meter) / (square(second) * volt),
              straightTrajectory.getDuration()), // trajectory
          minNode);                              // next volume node
    }

  } // namespace tracking_leapfrog_curved

} // namespace corsika
