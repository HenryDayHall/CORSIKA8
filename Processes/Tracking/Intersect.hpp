/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>
#include <corsika/geometry/Intersections.hpp>

#include <limits>

namespace corsika::process::tracking {

  /**
   * \class Intersect
   *
   * This is a CRTP class to provide a generic volume-tree
   * intersection for the purpose of tracking.
   *
   * It return the closest distance in time to the next geometric
   * intersection, as well as a pointer to the corresponding new
   * volume.
   *
   * User may provide an optional global step-length limit as
   * parameter. This may be needd for (simpler) algorithms in magnetic
   * fields, where tracking errors grow linearly with step-length.
   * Obviously, in the case of the step-length limit, the returend
   * "next" volume is just the current one.
   *
   **/

  template <typename TDerived>
  class Intersect {

  protected:
    template <typename TParticle>
    auto nextIntersect(
        const TParticle& particle,
        corsika::units::si::TimeType step_limit =
            std::numeric_limits<corsika::units::si::TimeType::value_type>::infinity() *
            corsika::units::si::second) const {
      using namespace corsika::units::si;
      using namespace corsika::geometry;

      typedef
          typename std::remove_reference<decltype(*particle.GetNode())>::type node_type;
      node_type& volumeNode =
          *particle.GetNode(); // current "logical" node, from previous tracking step
      C8LOG_DEBUG("volumeNode={}, numericallyInside={} ", fmt::ptr(&volumeNode),
                  volumeNode.GetVolume().Contains(particle.GetPosition()));

      // start values:
      TimeType minTime = step_limit;
      node_type* minNode = &volumeNode;

      // determine the first geometric collision with any other Volume boundary

      // first check, where we leave the current volume
      // this assumes our convention, that all Volume primitives must be convex
      // thus, the last entry is always the exit point
      const Intersections time_intersections_curr =
          TDerived::Intersect(particle, volumeNode);
      C8LOG_TRACE("curr node {}, parent node {} ", fmt::ptr(&volumeNode),
                  fmt::ptr(volumeNode.GetParent()));
      C8LOG_DEBUG("intersection times with currentLogicalVolumeNode: {} s and {} s",
                  time_intersections_curr.getEntry() / 1_s,
                  time_intersections_curr.getExit() / 1_s);
      if (time_intersections_curr.getExit() <= minTime) {
        minTime =
            time_intersections_curr.getExit(); // we exit currentLogicalVolumeNode here
        minNode = volumeNode.GetParent();
      }

      // where do we collide with any of the next-tree-level volumes
      // entirely contained by currentLogicalVolumeNode
      for (const auto& node : volumeNode.GetChildNodes()) {

        const Intersections time_intersections = TDerived::Intersect(particle, *node);
        C8LOG_DEBUG("intersection times with child volume {} : enter {} s, exit {} s",
                    fmt::ptr(node), time_intersections.getEntry() / 1_s,
                    time_intersections.getExit() / 1_s);

        const auto t_entry = time_intersections.getEntry();
        const auto t_exit = time_intersections.getExit();
        C8LOG_TRACE("children t-entry: {}, t-exit: {}, smaller? {} ", t_entry, t_exit,
                    t_entry <= minTime);
        // note, theoretically t can even be smaller than 0 since we
        // KNOW we can't yet be in this volume yet, so we HAVE TO
        // enter it IF exit point is not also in the "past"!
        if (t_exit > 0_s && t_entry <= minTime) { // enter volumen child here
          minTime = t_entry;
          minNode = node.get();
        }
      }

      // these are volumes from the previous tree-level that are cut-out partly from the
      // current volume
      for (node_type* node : volumeNode.GetExcludedNodes()) {

        const Intersections time_intersections = TDerived::Intersect(particle, *node);
        C8LOG_DEBUG("intersection times with exclusion volume {} : enter {} s, exit {} s",
                    fmt::ptr(node), time_intersections.getEntry() / 1_s,
                    time_intersections.getExit() / 1_s);
        const auto t_entry = time_intersections.getEntry();
        const auto t_exit = time_intersections.getExit();
        C8LOG_TRACE("children t-entry: {}, t-exit: {}, smaller? {} ", t_entry, t_exit,
                    t_entry <= minTime);
        // note, theoretically t can even be smaller than 0 since we
        // KNOW we can't yet be in this volume yet, so we HAVE TO
        // enter it IF exit point is not also in the "past"!
        if (t_exit > 0_s && t_entry <= minTime) { // enter volumen child here
          minTime = t_entry;
          minNode = node;
        }
      }
      C8LOG_TRACE("t-intersect: {}, node {} ", minTime, fmt::ptr(minNode));
      return std::make_tuple(minTime, minNode);
    }
  }; // namespace corsika::process::tracking
} // namespace corsika::process::tracking
