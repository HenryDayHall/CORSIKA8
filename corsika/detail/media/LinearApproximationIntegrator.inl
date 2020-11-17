/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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
  auto LinearApproximationIntegrator<TDerived>::integrateGrammage(
      Trajectory<Line> const& line, units::si::LengthType length) const {
    auto const c0 = getImplementation().evaluateAt(line.GetPosition(0));
    auto const c1 = getImplementation().rho_.FirstDerivative(line.GetPosition(0),
                                                              line.NormalizedDirection());
    return (c0 + 0.5 * c1 * length) * length;
  }

  template <typename TDerived>
  auto LinearApproximationIntegrator<TDerived>::arclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType grammage) const {
    auto const c0 = getImplementation().rho_(line.GetPosition(0));
    auto const c1 = getImplementation().rho_.FirstDerivative(line.GetPosition(0),
                                                             line.NormalizedDirection());

    return (1 - 0.5 * grammage * c1 / (c0 * c0)) * grammage / c0;
  }

  template <typename TDerived>
  auto LinearApproximationIntegrator<TDerived>::maximumLength(
      Trajectory<Line> const& line, [[maybe_unused]] double relError) const {
    using namespace units::si;
    [[maybe_unused]] auto const c1 = getImplementation().rho_.SecondDerivative(
        line.GetPosition(0), line.NormalizedDirection());

    // todo: provide a real, working implementation
    return 1_m * std::numeric_limits<double>::infinity();
  }

} // namespace corsika
