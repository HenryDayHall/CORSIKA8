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
#include <corsika/framework/geometry/BaseTrajectory.hpp>

namespace corsika {

  template <typename TDerived>
  class LinearApproximationIntegrator {
    auto const& getImplementation() const;

  public:
    auto getIntegrateGrammage(BaseTrajectory const& line) const;

    auto getArclengthFromGrammage(BaseTrajectory const& line,
                                  GrammageType grammage) const;

    auto getMaximumLength(BaseTrajectory const& line,
                          [[maybe_unused]] double relError) const;
  };

} // namespace corsika

#include <corsika/detail/media/LinearApproximationIntegrator.inl>
