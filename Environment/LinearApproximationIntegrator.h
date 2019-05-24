/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_environment_LinearApproximationIntegrator_h_
#define _include_environment_LinearApproximationIntegrator_h_

#include <limits>

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>

namespace corsika::environment {
  template <class TDerived>
  class LinearApproximationIntegrator {
    auto const& GetImplementation() const { return *static_cast<TDerived const*>(this); }

  public:
    auto IntegrateGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType length) const {
      auto const c0 = GetImplementation().EvaluateAt(line.GetPosition(0));
      auto const c1 = GetImplementation().fRho.FirstDerivative(
          line.GetPosition(0), line.NormalizedDirection());
      return (c0 + 0.5 * c1 * length) * length;
    }

    auto ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType grammage) const {
      auto const c0 = GetImplementation().fRho(line.GetPosition(0));
      auto const c1 = GetImplementation().fRho.FirstDerivative(
          line.GetPosition(0), line.NormalizedDirection());

      return (1 - 0.5 * grammage * c1 / (c0 * c0)) * grammage / c0;
    }

    auto MaximumLength(corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
                       [[maybe_unused]] double relError) const {
      using namespace corsika::units::si;
      [[maybe_unused]] auto const c1 = GetImplementation().fRho.SecondDerivative(
          line.GetPosition(0), line.NormalizedDirection());

      // todo: provide a real, working implementation
      return 1_m * std::numeric_limits<double>::infinity();
    }
  };
} // namespace corsika::environment

#endif
