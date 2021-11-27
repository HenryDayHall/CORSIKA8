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
#include <corsika/framework/geometry/Cubic.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/geometry/StraightTrajectory.hpp>
#include <corsika/framework/geometry/Intersections.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/modules/tracking/Intersect.hpp>

#include <type_traits>
#include <utility>

namespace corsika::tracking_line {

  /**
   * Tracking of particles without charge or in no magnetic fields.
   *
   * This will simply follow traight lines, but intersect them with
   * the geometry volume model of the environment.
   *
   **/

  class Tracking : public Intersect<Tracking> {

    using Intersect<Tracking>::nextIntersect;

  public:
    /**
     * Performs one straight step of length steplength.
     */
    template <typename TParticle>
    static auto makeStep(TParticle const& particle, LengthType const steplength);

    //! Determine track of particle
    template <typename TParticle>
    auto getTrack(TParticle const& particle);

    //! find intersection of Sphere with Track
    template <typename TParticle>
    static Intersections intersect(TParticle const& particle, Sphere const& sphere);

    template <typename TParticle>
    static Intersections intersect(TParticle const& particle, Cubic const& cubic);

    //! find intersection of Volume node with Track of particle
    template <typename TParticle, typename TBaseNodeType>
    static Intersections intersect(TParticle const& particle, TBaseNodeType const& node);

    //! find intersection of Plane with Track
    template <typename TParticle>
    static Intersections intersect(TParticle const& particle, Plane const& plane);

    static std::string getName() { return "Tracking-Straight"; }
    static std::string getVersion() { return "1.0.0"; }
  };

} // namespace corsika::tracking_line

#include <corsika/detail/modules/tracking/TrackingStraight.inl>
