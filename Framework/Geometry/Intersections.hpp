/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/units/PhysicalUnits.h>

#include <map> // for pair

namespace corsika::geometry {

  /**
   * \class Intersection
   *
   * Container to store and return a list of intersections of a
   * trajectory with a geometric volume objects in space.
   *
   **/

  class Intersections {

    Intersections(const Intersections&) = delete;
    Intersections(Intersections&&) = delete;
    Intersections& operator=(const Intersections&) = delete;

  public:
    Intersections()
        : has_intersections_(false) {}
    Intersections(corsika::units::si::TimeType&& t1, corsika::units::si::TimeType&& t2)
        : has_intersections_(true)
        , intersections_(std::make_pair(t1, t2)) {}
    Intersections(corsika::units::si::TimeType&& t)
        : has_intersections_(true)
        , intersections_(std::make_pair(
              t,
              std::numeric_limits<corsika::units::si::TimeType::value_type>::infinity() *
                  corsika::units::si::second)) {}

    bool hasIntersections() const { return has_intersections_; }
    ///! where did the trajectory currently enter the volume
    corsika::units::si::TimeType getEntry() const { return intersections_.first; }
    ///! where did the trajectory currently exit the volume
    corsika::units::si::TimeType getExit() const { return intersections_.second; }

  private:
    bool has_intersections_;
    std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType> intersections_;
  };

} // namespace corsika::geometry
