/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/radio/propagators/SignalPath.hpp>

namespace corsika {

  inline SignalPath::SignalPath(
      const TimeType propagation_time, const double average_refractive_index,
      const double refractive_index_source, const double refractive_index_destination,
      const Vector<dimensionless_d> emit, const Vector<dimensionless_d> receive,
      const LengthType R_distance, const std::deque<Point>& points)
      : Path(points)
      , propagation_time_(propagation_time)
      , average_refractive_index_(average_refractive_index)
      , refractive_index_source_(refractive_index_source)
      , refractive_index_destination_(refractive_index_destination)
      , emit_(emit)
      , receive_(receive)
      , R_distance_(R_distance) {}

} // namespace corsika