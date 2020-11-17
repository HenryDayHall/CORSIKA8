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
    auto integrateGrammage(Trajectory<Line> const& line,
                           units::si::LengthType length) const;

    auto arclengthFromGrammage(Trajectory<Line> const& line,
                               units::si::GrammageType grammage) const;

    auto maximumLength(Trajectory<Line> const& line,
                       [[maybe_unused]] double relError) const;
  };

} // namespace corsika

#include <corsika/detail/media/LinearApproximationIntegrator.inl>
