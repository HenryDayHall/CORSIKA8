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
      auto magneticfield =
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
      auto steplength_true = steplength * (1.0 + double(direction.norm())) / 2;
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

        // traverse the environment volume tree and find next
        // intersection
        auto [minTime, minNode] = tracking::Intersect<Tracking>::nextIntersect(particle);

        const int chargeNumber = particle.GetChargeNumber();
        const auto& magneticfield =
            volumeNode.GetModelProperties().GetMagneticField(position);
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
        const int chargeNumber = particle.GetChargeNumber();
        const auto& position = particle.GetPosition();
        const auto& magneticfield = medium.GetMagneticField(position);

        if (chargeNumber == 0 || magneticfield.norm() == 0_T) {
          return tracking_line::Tracking::Intersect(particle, sphere, medium);
        }

        const geometry::Vector<SpeedType::dimension_type> velocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
        const auto absVelocity = velocity.norm();
        auto energy = particle.GetEnergy();
        auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV /
                 (absVelocity * energy * 1_V);
        const geometry::Vector<dimensionless_d> directionBefore =
            velocity.normalized(); // determine steplength to next volume

        const double a =
            ((directionBefore.cross(magneticfield)).dot(position - sphere.GetCenter()) *
                 k +
             1) *
            4 /
            (1_m * 1_m * (directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k);
        const double b = directionBefore.dot(position - sphere.GetCenter()) * 8 /
                         ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k *
                          k * 1_m * 1_m * 1_m);
        const double c = ((position - sphere.GetCenter()).GetSquaredNorm() -
                          (sphere.GetRadius() * sphere.GetRadius())) *
                         4 /
                         ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k *
                          k * 1_m * 1_m * 1_m * 1_m);
        std::complex<double>* solutions = solve_quartic(0, a, b, c);
        LengthType d_enter, d_exit;
        int first = 0;
        for (int i = 0; i < 4; i++) {
          if (solutions[i].imag() == 0) {
            LengthType time = solutions[i].real() * 1_m;
            C8LOG_TRACE("Solutions for current Volume: {} ", time);
            if (first == 0) {
              d_enter = time;
            } else {
              if (time < d_enter) {
                d_exit = d_enter;
                d_enter = time;
              } else {
                d_exit = time;
              }
            }
            first++;
          }
        }
        delete[] solutions;

        if (first != 2) {
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
    };

  } // namespace tracking_leapfrog_curved

} // namespace corsika::process
