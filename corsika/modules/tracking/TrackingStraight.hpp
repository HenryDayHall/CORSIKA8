/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/framework/geometry/Intersections.hpp>
#include <corsika/framework/logging/Logging.hpp>
#include <corsika/modules/tracking/Intersect.hpp>

#include <type_traits>
#include <utility>

namespace corsika::tracking_line {

  /**
   * \class Tracking
   *
   *
   *
   **/

  class Tracking : public Intersect<Tracking> {

    using Intersect<Tracking>::nextIntersect;

  public:
    template <typename TParticle>
    auto getTrack(TParticle const& particle) {
      VelocityVector const initialVelocity =
          particle.getMomentum() / particle.getEnergy() * constants::c;

      auto const initialPosition = particle.getPosition();
      CORSIKA_LOG_DEBUG(
          "Tracking pid: {}"
          " , E = {} GeV",
          particle.getPID(), particle.getEnergy() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking pos: {}", initialPosition.getCoordinates());
      CORSIKA_LOG_DEBUG("Tracking   E: {} GeV", particle.getEnergy() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking   p: {} GeV",
                        particle.getMomentum().getComponents() / 1_GeV);
      CORSIKA_LOG_DEBUG("Tracking   v: {} ", initialVelocity.getComponents());

      // traverse the environment volume tree and find next
      // intersection
      auto [minTime, minNode] = nextIntersect(particle);

      return std::make_tuple(LineTrajectory(Line(initialPosition, initialVelocity),
                                            minTime), // trajectory
                             minNode);                // next volume node
    }

    template <typename TParticle, typename TMedium>
    static Intersections intersect(TParticle const& particle, Sphere const& sphere,
                                   TMedium const&) {
      auto const delta = particle.getPosition() - sphere.getCenter();
      auto const velocity = particle.getMomentum() / particle.getEnergy() * constants::c;
      auto const vSqNorm = velocity.getSquaredNorm();
      auto const R = sphere.getRadius();

      auto const vDotDelta = velocity.dot(delta);
      auto const discriminant =
          vDotDelta * vDotDelta - vSqNorm * (delta.getSquaredNorm() - R * R);

      if (discriminant.magnitude() > 0) {
        auto const sqDisc = sqrt(discriminant);
        auto const invDenom = 1 / vSqNorm;
        return Intersections((-vDotDelta - sqDisc) * invDenom,
                             (-vDotDelta + sqDisc) * invDenom);
      }
      return Intersections();
    }

    template <typename TParticle, typename TBaseNodeType>
    static Intersections intersect(TParticle const& particle,
                                   TBaseNodeType const& volumeNode) {
      Sphere const* sphere = dynamic_cast<Sphere const*>(&volumeNode.getVolume());
      if (sphere) {
        return intersect(particle, *sphere, volumeNode.getModelProperties());
      }
      throw std::runtime_error(
          "The Volume type provided is not supported in Intersect(particle, node)");
    }

    template <typename TParticle, typename TMedium>
    static Intersections intersect(TParticle const& particle, Plane const& plane,
                                   TMedium const&) {
      auto const delta = plane.getCenter() - particle.getPosition();
      auto const velocity = particle.getMomentum() / particle.getEnergy() * constants::c;
      auto const n = plane.getNormal();
      auto const c = n.dot(velocity);

      return Intersections(
          c.magnitude() == 0 ? std::numeric_limits<TimeType::value_type>::infinity() * 1_s
                             : n.dot(delta) / c);
    }
  };

} // namespace corsika::tracking_line

// #include <corsika/detail/modules/tracking/TrackingLine.inl>
