/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/tracking_line/Tracking.h>
#include <corsika/process/tracking/Intersect.hpp>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/geometry/Vector.h>
#include <corsika/geometry/Intersections.hpp>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/quartic.h>
#include <corsika/logging/Logging.h>

#include <type_traits>
#include <utility>

#include <fstream>

namespace corsika::process {

  namespace tracking_leapfrog_curved {

    typedef corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>
        MagneticFieldVector;

    /**
     * \function LeapFrogStep
     *
     * Performs one leap-frog step consistent of two halve-steps with steplength/2
     * The step is caluculated analytically precisely to reach to the next volume
     *boundary.
     **/
    template <typename TParticle>
    auto LeapFrogStep(const TParticle& particle,
                      corsika::units::si::LengthType steplength) {
      using namespace corsika::units::si;
      if (particle.GetMomentum().norm() == 0_GeV) {
        return std::make_tuple(particle.GetPosition(), particle.GetMomentum() / 1_GeV,
                               double(0));
      } // charge of the particle
      const int chargeNumber = particle.GetChargeNumber();
      auto const* currentLogicalVolumeNode = particle.GetNode();
      MagneticFieldVector const& magneticfield =
          currentLogicalVolumeNode->GetModelProperties().GetMagneticField(
              particle.GetPosition());
      geometry::Vector<SpeedType::dimension_type> velocity =
          particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
      decltype(corsika::units::si::meter /
               (corsika::units::si::second * corsika::units::si::volt)) k =
          chargeNumber * corsika::units::constants::cSquared * 1_eV /
          (velocity.norm() * particle.GetEnergy() * 1_V);
      geometry::Vector<dimensionless_d> direction = velocity.normalized();
      auto position = particle.GetPosition(); // First Movement
      // assuming magnetic field does not change during movement
      position =
          position + direction * steplength / 2; // Change of direction by magnetic field
      direction =
          direction + direction.cross(magneticfield) * steplength * k; // Second Movement
      position = position + direction * steplength / 2;
      auto steplength_true = steplength * (1.0 + (double)direction.norm()) / 2;
      return std::make_tuple(position, direction.normalized(), steplength_true);
    }

    /**
     * \class Tracking
     *
     * The class tracking_leapfrog_curved::Tracking is based on the
     * Bachelor thesis of Andre Schmidt (KIT). It implements a
     * two-step leap-frog algorithm, but with analytically exact geometric
     * intersections between leap-frog steps and geometric volumes
     * (spheres, planes).
     *
     **/

    class Tracking : public corsika::process::tracking::Intersect<Tracking> {

    public:
      Tracking()
          : straightTracking_{tracking_line::Tracking()} {}

      template <typename TParticle>
      auto GetTrack(TParticle const& particle) {
        using namespace corsika::units::si;
        using namespace corsika::geometry;
        geometry::Vector<SpeedType::dimension_type> const initialVelocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;

        auto const position = particle.GetPosition();
        C8LOG_DEBUG(
            "Tracking pid: {}"
            " , E = {} GeV",
            particle.GetPID(), particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("Tracking pos: {}", position.GetCoordinates());
        C8LOG_DEBUG("Tracking   E: {} GeV", particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("Tracking   p: {} GeV",
                    particle.GetMomentum().GetComponents() / 1_GeV);
        C8LOG_DEBUG("Tracking   v: {} ", initialVelocity.GetComponents());

        typedef
            typename std::remove_reference<decltype(*particle.GetNode())>::type node_type;
        node_type& volumeNode = *particle.GetNode();

        // for the event of magnetic fields and curved trajectories, we need to limit
        // maximum step-length since we need to follow curved
        // trajectories segment-wise -- at least if we don't employ concepts as "Helix
        // Trajectories" or similar
        MagneticFieldVector const& magneticfield =
            volumeNode.GetModelProperties().GetMagneticField(position);
        corsika::units::si::MagneticFluxType const magnitudeB = magneticfield.norm();
        int const chargeNumber = particle.GetChargeNumber();
        bool const no_deflection = chargeNumber == 0 || magnitudeB == 0_T;

        if (no_deflection) { return GetLinearTrajectory(particle); }

        HEPMomentumType const pAlongB_delta =
            (particle.GetMomentum() -
             particle.GetMomentum().parallelProjectionOnto(magneticfield))
                .norm();

        if (pAlongB_delta == 0_GeV) {
          // particle travel along, parallel to magnetic field. Rg is
          // "0", but for purpose of step limit we return infinity here.
          C8LOG_TRACE("pAlongB_delta is 0_GeV --> parallel");
          return GetLinearTrajectory(particle);
        }

        LengthType const gyroradius =
            (pAlongB_delta * 1_V /
             (corsika::units::constants::c * abs(chargeNumber) * magnitudeB * 1_eV));

        const double maxRadians = 0.01;
        const LengthType steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
        const TimeType steplimit_time = steplimit / initialVelocity.norm();
        C8LOG_DEBUG("gyroradius {}, steplimit: {} = {}", gyroradius, steplimit,
                    steplimit_time);

        // traverse the environment volume tree and find next
        // intersection
        auto [minTime, minNode] =
            tracking::Intersect<Tracking>::nextIntersect(particle, steplimit_time);

        const auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV /
                       (particle.GetEnergy() * 1_V);
        return std::make_tuple(
            geometry::LeapFrogTrajectory(position, initialVelocity, magneticfield, k,
                                         minTime), // trajectory
            minNode);                              // next volume node
      }

      template <typename TParticle, typename TMedium>
      static geometry::Intersections Intersect(const TParticle& particle,
                                               const corsika::geometry::Sphere& sphere,
                                               const TMedium& medium) {
        using namespace corsika::units::si;

        if (sphere.GetRadius() == 1_km * std::numeric_limits<double>::infinity()) {
          return geometry::Intersections();
        }

        const int chargeNumber = particle.GetChargeNumber();
        const auto& position = particle.GetPosition();
        MagneticFieldVector const& magneticfield = medium.GetMagneticField(position);

        const geometry::Vector<SpeedType::dimension_type> velocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
        const geometry::Vector<dimensionless_d> directionBefore =
            velocity.normalized(); // determine steplength to next volume

        auto const projectedDirection = directionBefore.cross(magneticfield);
        auto const projectedDirectionSqrNorm = projectedDirection.GetSquaredNorm();
        bool const isParallel = (projectedDirectionSqrNorm == 0 * square(1_T));

        if (chargeNumber == 0 || magneticfield.norm() == 0_T || isParallel) {
          return tracking_line::Tracking::Intersect(particle, sphere, medium);
        }

        bool const numericallyInside = sphere.Contains(particle.GetPosition());

        const auto absVelocity = velocity.norm();
        auto energy = particle.GetEnergy();
        auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV /
                 (absVelocity * energy * 1_V);

        auto const denom =
            (directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k;
        const double a =
            ((directionBefore.cross(magneticfield)).dot(position - sphere.GetCenter()) *
                 k +
             1) *
            4 / (1_m * 1_m * denom);
        const double b = directionBefore.dot(position - sphere.GetCenter()) * 8 /
                         (denom * 1_m * 1_m * 1_m);
        const double c = ((position - sphere.GetCenter()).GetSquaredNorm() -
                          (sphere.GetRadius() * sphere.GetRadius())) *
                         4 / (denom * 1_m * 1_m * 1_m * 1_m);
        C8LOG_TRACE("denom={}, a={}, b={}, c={}", denom, a, b, c);
        std::complex<double>* solutions = solve_quartic(0, a, b, c);
        LengthType d_enter, d_exit;
        int first = 0, first_entry = 0, first_exit = 0;
        for (int i = 0; i < 4; i++) {
          if (solutions[i].imag() == 0) {
            LengthType const dist = solutions[i].real() * 1_m;
            C8LOG_TRACE("Solution (real) for current Volume: {} ", dist);
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
          C8LOG_DEBUG("no intersection! count={}", first);
          return geometry::Intersections();
        }
        return geometry::Intersections(d_enter / absVelocity, d_exit / absVelocity);
      }

      template <typename TParticle, typename TBaseNodeType>
      static geometry::Intersections Intersect(const TParticle& particle,
                                               const TBaseNodeType& volumeNode) {
        const geometry::Sphere* sphere =
            dynamic_cast<const geometry::Sphere*>(&volumeNode.GetVolume());
        if (sphere) {
          return Intersect(particle, *sphere, volumeNode.GetModelProperties());
        }
        throw std::runtime_error(
            "The Volume type provided is not supported in Intersect(particle, node)");
      }

    protected:
      /**
       * Use internally stored class tracking_line::Tracking to
       * perform a straight line tracking, if no magnetic bendig was
       * detected.
       *
       */
      template <typename TParticle>
      auto GetLinearTrajectory(TParticle& particle) {

        using namespace corsika::units::si;

        // perform simple linear tracking
        auto [straightTrajectory, minNode] = straightTracking_.GetTrack(particle);

        // return as leap-frog trajectory
        return std::make_tuple(
            geometry::LeapFrogTrajectory(
                straightTrajectory.GetLine().GetR0(),
                straightTrajectory.GetLine().GetV0(),
                MagneticFieldVector(particle.GetPosition().GetCoordinateSystem(), 0_T,
                                    0_T, 0_T),
                square(0_m) / (square(1_s) * 1_V),
                straightTrajectory.GetDuration()), // trajectory
            minNode);                              // next volume node
      }

    protected:
      tracking_line::Tracking
          straightTracking_; ///! we want this for neutral and B=0T tracks

    }; // namespace tracking_leapfrog_curved

  } // namespace tracking_leapfrog_curved

} // namespace corsika::process
