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

    /**
     * \file TrackingLeapFrogCurved.hpp The leap-frog tracking.
     */

    /**
     * The class tracking_leapfrog_curved::Tracking is based on the
     * Bachelor thesis of Andre Schmidt (KIT). It implements a
     * two-step leap-frog algorithm, but with analytically exact geometric
     * intersections between leap-frog steps and geometric volumes
     * (spheres, planes).
     */

    class Tracking : public Intersect<Tracking> {

      using Intersect<Tracking>::nextIntersect;

    public:
      Tracking()
          : straightTracking_{tracking_line::Tracking()} {}

      template <typename TParticle>
      auto getTrack(TParticle const& particle);

      /**
       * Performs one leap-frog step consistent of two halve-steps with steplength/2
       * Due to the nature of the algorithm the second halve step is slightly longer than
       * the first halve step.
       */
      template <typename TParticle>
      static auto makeStep(TParticle const& particle, LengthType const steplength);

      /**
       *  find intersection of Sphere with Track
       *
       * Returns intersection of particle assuming a curved leap-frog step, with a sphere.
       * Entry and exit times are calculated, where the velocity is constant and the
       * steplength is the geometric steplength of the leap-frog.
       *
       * @param particle Current particle state
       * @param sphere Sphere object
       */
      template <typename TParticle>
      static Intersections intersect(TParticle const& particle, Sphere const& sphere);

      /**
       *  find intersection of any Volume node with particle
       *
       * The intersection time(s) of a particle, assuming a curved leap-frog
       * step, are calculated for any volume type.
       */
      template <typename TParticle, typename TBaseNodeType>
      static Intersections intersect(TParticle const& particle,
                                     TBaseNodeType const& node);

      /**
       *  find intersection of Plane with Track
       *
       * Intersection times of particle are caculated with a plane, assuming a curved leap
       * frog trajectory. The intersection time is assuming constant velocity (no change)
       * along the geometric leap-frog step.
       *
       * @tparam TParticle Type of particle object on stack.
       * @param particle Particle initial state.
       * @param plane Plane.
       * @return Intersections in time units.
       */
      template <typename TParticle>
      static Intersections intersect(TParticle const& particle, Plane const& plane);

      static std::string getName() { return "LeapFrog-curved"; }
      static std::string getVersion() { return "1.0.0"; }

    protected:
      /**
       * Use internally stored class tracking_line::Tracking to
       * perform a straight line tracking, if no magnetic bendig was
       * detected.
       */
      template <typename TParticle>
      auto getLinearTrajectory(TParticle& particle);

    protected:
      tracking_line::Tracking
          straightTracking_; ///! we want this for neutral and B=0T tracks

    }; // namespace tracking_leapfrog_curved

  } // namespace tracking_leapfrog_curved

} // namespace corsika

#include <corsika/detail/modules/tracking/TrackingLeapFrogCurved.inl>
