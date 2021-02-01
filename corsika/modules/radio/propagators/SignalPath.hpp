/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Path.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  /**
   * Store the photon signal path between two points.
   *
   * This is basically a container class
   */
  struct SignalPath final : private Path {

    using path = std::deque<Point>;

    TimeType const time_;    ///< The total propagation time.
    double const average_refractivity_; ///< The average refractivity.
    Vector<dimensionless_d> const emit_;    ///< The (unit-length) emission vector.
    Vector<dimensionless_d> const receive_; ///< The (unit-length) receive vector.
    path const points_;  ///< A collection of points that make up the geometrical path.

    /**
     * Create a new SignalPath instance.
     */
    SignalPath(TimeType const time, double const average_refractivity,
               Vector<dimensionless_d> const emit, Vector<dimensionless_d> const receive,
               path const& points)
        : Path(points)
        , time_(time)
        , average_refractivity_(average_refractivity)
        , emit_(emit)
        , receive_(receive) {}

  }; // class SignalPath

} // namespace corsika
