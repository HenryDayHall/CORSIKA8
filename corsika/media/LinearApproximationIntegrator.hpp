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
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {

  template <typename TDerived>
  class LinearApproximationIntegrator {
    auto const& getImplementation() const;

  public:
    auto getIntegrateGrammage(setup::Trajectory const& line, LengthType length) const;

    auto getArclengthFromGrammage(setup::Trajectory const& line,
                                  GrammageType grammage) const;

    auto getMaximumLength(setup::Trajectory const& line,
                          [[maybe_unused]] double relError) const;
  };

} // namespace corsika

#include <corsika/detail/media/LinearApproximationIntegrator.inl>
