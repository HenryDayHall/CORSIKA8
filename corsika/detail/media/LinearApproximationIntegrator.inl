/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/LinearApproximationIntegrator.hpp>

namespace corsika {

  template <typename TDerived>
  auto const& LinearApproximationIntegrator<TDerived>::getImplementation() const {
    return *static_cast<TDerived const*>(this);
  }

  template <typename TDerived>
  auto LinearApproximationIntegrator<TDerived>::getIntegrateGrammage(
      setup::Trajectory const& line, LengthType length) const {
    auto const c0 = getImplementation().evaluateAt(line.getPosition(0));
    auto const c1 = getImplementation().rho_.getFirstDerivative(line.getPosition(0),
                                                                line.getDirection(0));
    return (c0 + 0.5 * c1 * length) * length;
  }

  template <typename TDerived>
  auto LinearApproximationIntegrator<TDerived>::getArclengthFromGrammage(
      setup::Trajectory const& line, GrammageType grammage) const {
    auto const c0 = getImplementation().rho_(line.getPosition(0));
    auto const c1 = getImplementation().rho_.getFirstDerivative(line.getPosition(0),
                                                                line.getDirection(0));

    return (1 - 0.5 * grammage * c1 / (c0 * c0)) * grammage / c0;
  }

  template <typename TDerived>
  auto LinearApproximationIntegrator<TDerived>::getMaximumLength(
      setup::Trajectory const& line, [[maybe_unused]] double relError) const {
    [[maybe_unused]] auto const c1 = getImplementation().rho_.getSecondDerivative(
        line.getPosition(0), line.getDirection(0));

    // todo: provide a real, working implementation
    return 1_m * std::numeric_limits<double>::infinity();
  }

} // namespace corsika
