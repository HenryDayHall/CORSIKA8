/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Intersections.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/StraightTrajectory.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/modules/tracking/Intersect.hpp>

#include <cmath>
#include <type_traits>
#include <utility>

namespace corsika::tracking_line {

template <typename TParticle>
inline auto Tracking::makeStep(TParticle const &particle,
                               LengthType steplength) {
  if (particle.getMomentum().getNorm() == 0_GeV) {
    return std::make_tuple(particle.getPosition(),
                           particle.getMomentum() / 1_GeV);
  } // charge of the particle
  DirectionVector const dir = particle.getDirection();
  return std::make_tuple(particle.getPosition() + dir * steplength,
                         dir.normalized());
}

template <typename TParticle>
inline auto Tracking::getTrack(TParticle const &particle) {
  VelocityVector const initialVelocity =
      particle.getMomentum() / particle.getEnergy() * constants::c;

  auto const &initialPosition = particle.getPosition();
  CORSIKA_LOG_DEBUG("TrackingStraight pid: {}"
                    " , E = {} GeV \n"
                    "\tTracking pos: {} \n"
                    "\tTracking   p: {} GeV \n"
                    "\tTracking   v: {}",
                    particle.getPID(), particle.getEnergy() / 1_GeV,
                    initialPosition.getCoordinates(),
                    particle.getMomentum().getComponents() / 1_GeV,
                    initialVelocity.getComponents());

  // traverse the environment volume tree and find next
  // intersection
  auto [minTime, minNode] = nextIntersect(particle);

  return std::make_tuple(
      StraightTrajectory(Line(initialPosition, initialVelocity),
                         minTime), // trajectory
      minNode);                    // next volume node
}

template <typename TParticle>
inline Intersections Tracking::intersect(TParticle const &particle,
                                         Sphere const &sphere) {
  auto const &position = particle.getPosition();
  auto const delta = position - sphere.getCenter();
  auto const velocity =
      particle.getMomentum() / particle.getEnergy() * constants::c;
  auto const vSqNorm = velocity.getSquaredNorm();
  auto const R = sphere.getRadius();

  auto const vDotDelta = velocity.dot(delta);
  auto const discriminant =
      vDotDelta * vDotDelta - vSqNorm * (delta.getSquaredNorm() - R * R);

  if (discriminant.magnitude() > 0) {
    auto const sqDisc = sqrt(discriminant);
    auto const invDenom = 1 / vSqNorm;

    CORSIKA_LOG_TRACE("numericallyInside={}", sphere.contains(position));
    return Intersections((-vDotDelta - sqDisc) * invDenom,
                         (-vDotDelta + sqDisc) * invDenom);
  }
  return Intersections();
}

template <typename TParticle>
inline Intersections Tracking::intersect(TParticle const &particle,
                                         Cubic const &cubic) {
  Point const &position = particle.getPosition();
  VelocityVector const velocity =
      particle.getMomentum() / particle.getEnergy() * constants::c;
  CoordinateSystemPtr const &cs = cubic.getCoordinateSystem();
  LengthType x0 = position.getX(cs);
  LengthType y0 = position.getY(cs);
  LengthType z0 = position.getZ(cs);
  SpeedType vx = velocity.getX(cs);
  SpeedType vy = velocity.getY(cs);
  SpeedType vz = velocity.getZ(cs);

  auto get_intersect_min_max = [](LengthType x0, SpeedType v0, LengthType dx) {
    auto t1 = (x0 - dx) / v0;
    auto t2 = (x0 + dx) / v0;
    if (t1 > t2)
      return std::make_pair(t1, t2);
    else
      return std::make_pair(t2, t1);
  };

  auto [tx_max, tx_min] = get_intersect_min_max(x0, vx, cubic.getX());
  auto [ty_max, ty_min] = get_intersect_min_max(y0, vy, cubic.getY());
  auto [tz_max, tz_min] = get_intersect_min_max(z0, vz, cubic.getZ());

  TimeType t_exit = std::min(std::min(tx_max, ty_max), tz_max);
  TimeType t_enter = std::min(std::min(tx_min, ty_min), tz_min);

  if ((t_exit > t_enter)) {
    if (t_enter < 0_s && t_exit > 0_s)
      CORSIKA_LOG_DEBUG("numericallyInside={}", cubic.contains(position));
    else if (t_enter < 0_s && t_exit < 0_s)
      CORSIKA_LOG_DEBUG("oppisite direction");
    return Intersections(std::move(t_enter), std::move(t_exit));
  } else
    return Intersections();
}

template <typename TParticle, typename TBaseNodeType>
inline Intersections Tracking::intersect(TParticle const &particle,
                                         TBaseNodeType const &volumeNode) {
  if (Sphere const *sphere =
          dynamic_cast<Sphere const *>(&volumeNode.getVolume());
      sphere) {
    return Tracking::intersect<TParticle>(particle, *sphere);
  } else if (Cubic const *cubic =
                 dynamic_cast<Cubic const *>(&volumeNode.getVolume());
             cubic) {
    return Tracking::intersect<TParticle>(particle, *cubic);
  } else {
    throw std::runtime_error("The Volume type provided is not supported in "
                             "Intersect(particle, node)");
  }
}

template <typename TParticle>
inline Intersections Tracking::intersect(TParticle const &particle,
                                         Plane const &plane) {
  auto const delta = plane.getCenter() - particle.getPosition();
  auto const velocity =
      particle.getMomentum() / particle.getEnergy() * constants::c;
  auto const n = plane.getNormal();
  auto const n_dot_v = n.dot(velocity);

  CORSIKA_LOG_TRACE("n_dot_v={}, delta={}, momentum={}", n_dot_v, delta,
                    particle.getMomentum());

  if (n_dot_v.magnitude() == 0)
    return Intersections();
  else
    return Intersections(n.dot(delta) / n_dot_v);
}

} // namespace corsika::tracking_line
