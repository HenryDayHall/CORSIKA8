/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>
#include <corsika/geometry/Intersections.hpp>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>
#include <corsika/process/tracking/Intersect.hpp>
#include <corsika/geometry/Trajectory.h>

#include <type_traits>
#include <utility>

namespace corsika::process {

  namespace tracking_line {

    /**
     * \class Tracking
     *
     *
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

        auto const initialPosition = particle.GetPosition();
        C8LOG_DEBUG(
            "Tracking pid: {}"
            " , E = {} GeV",
            particle.GetPID(), particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("Tracking pos: {}", initialPosition.GetCoordinates());
        C8LOG_DEBUG("Tracking   E: {} GeV", particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("Tracking   p: {} GeV",
                    particle.GetMomentum().GetComponents() / 1_GeV);
        C8LOG_DEBUG("Tracking   v: {} ", initialVelocity.GetComponents());

        // traverse the environment volume tree and find next
        // intersection
        auto [minTime, minNode] = tracking::Intersect<Tracking>::nextIntersect(particle);

        return std::make_tuple(
            geometry::LineTrajectory(geometry::Line(initialPosition, initialVelocity),
                                     minTime), // trajectory
            minNode);                          // next volume node
      }

      template <typename TParticle, typename TMedium>
      static geometry::Intersections Intersect(const TParticle& particle,
                                               const corsika::geometry::Sphere& sphere,
                                               const TMedium&) {
        using namespace corsika::units::si;
        auto const delta = particle.GetPosition() - sphere.GetCenter();
        auto const velocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
        auto const vSqNorm = velocity.squaredNorm();
        auto const R = sphere.GetRadius();

        auto const vDotDelta = velocity.dot(delta);
        auto const discriminant =
            vDotDelta * vDotDelta - vSqNorm * (delta.squaredNorm() - R * R);

        if (discriminant.magnitude() > 0) {
          auto const sqDisc = sqrt(discriminant);
          auto const invDenom = 1 / vSqNorm;
          return geometry::Intersections((-vDotDelta - sqDisc) * invDenom,
                                         (-vDotDelta + sqDisc) * invDenom);
        }
        return geometry::Intersections();
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

      template <typename TParticle, typename TMedium>
      static geometry::Intersections Intersect(const TParticle& particle,
                                               const geometry::Plane& plane,
                                               const TMedium&) {
        using namespace corsika::units::si;
        auto const delta = plane.GetCenter() - particle.GetPosition();
        auto const velocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
        auto const n = plane.GetNormal();
        auto const c = n.dot(velocity);

        return Intersections(c.magnitude() == 0
                                 ? std::numeric_limits<TimeType::value_type>::infinity() *
                                       1_s
                                 : n.dot(delta) / c);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process
