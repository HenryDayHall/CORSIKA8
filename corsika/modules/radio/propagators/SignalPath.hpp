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

    //TODO: discuss if we need average refractivity or average refractive index
    TimeType const propagation_time_;    ///< The total propagation time.
    double const average_refractive_index_; ///< The average refractive index.
    double const refractive_index_source_; ///< The refractive index at the source.
    Vector<dimensionless_d> const emit_;    ///< The (unit-length) emission vector.
    Vector<dimensionless_d> const receive_; ///< The (unit-length) receive vector.
    path const points_;  ///< A collection of points that make up the geometrical path.
    LengthType const R_distance_; ///< The distance from the point of emission to an observer. TODO: optical path, not geometrical! (probably)

    /**
     * Create a new SignalPath instance.
     */
    SignalPath(TimeType const propagation_time, double const average_refractive_index, double const refractive_index_source,
               Vector<dimensionless_d> const emit, Vector<dimensionless_d> const receive,
               LengthType const R_distance, path const& points)
        : Path(points)
        , propagation_time_(propagation_time)
        , average_refractive_index_(average_refractive_index)
        , refractive_index_source_(refractive_index_source)
        , emit_(emit)
        , receive_(receive)
        , R_distance_(R_distance) {}

  }; // class SignalPath

} // namespace corsika
