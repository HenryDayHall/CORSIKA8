/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <limits>

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

namespace corsika {

  template <typename TDerived>
  class LinearApproximationIntegrator {
    auto const& getImplementation() const;

  public:
    auto getIntegrateGrammage(Trajectory<Line> const& line, LengthType length) const;

    auto getArclengthFromGrammage(Trajectory<Line> const& line,
                                  GrammageType grammage) const;

    auto getMaximumLength(Trajectory<Line> const& line,
                          [[maybe_unused]] double relError) const;
  };

} // namespace corsika

#include <corsika/detail/media/LinearApproximationIntegrator.inl>
