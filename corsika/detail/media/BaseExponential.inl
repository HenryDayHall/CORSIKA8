/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/BaseExponential.hpp>

namespace corsika {

  template <class TDerived>
  auto const& BaseExponential<TDerived>::GetImplementation() const {
    return *static_cast<TDerived const*>(this);
  }

  template <class TDerived>
  GrammageType BaseExponential<TDerived>::IntegratedGrammage(
      Trajectory<Line> const& vLine, LengthType vL,
      Vector<dimensionless_d> const& vAxis) const {
    if (vL == LengthType::zero()) { return GrammageType::zero(); }

    auto const uDotA = vLine.NormalizedDirection().dot(vAxis).magnitude();
    auto const rhoStart = GetImplementation().GetMassDensity(vLine.GetR0());

    if (uDotA == 0) {
      return vL * rhoStart;
    } else {
      return rhoStart * (fLambda / uDotA) * (exp(uDotA * vL * fInvLambda) - 1);
    }
  }

  template <class TDerived>
  LengthType BaseExponential<TDerived>::ArclengthFromGrammage(
      Trajectory<Line> const& vLine, GrammageType vGrammage,
      Vector<dimensionless_d> const& vAxis) const {
    auto const uDotA = vLine.NormalizedDirection().dot(vAxis).magnitude();
    auto const rhoStart = GetImplementation().GetMassDensity(vLine.GetR0());

    if (uDotA == 0) {
      return vGrammage / rhoStart;
    } else {
      auto const logArg = vGrammage * fInvLambda * uDotA / rhoStart + 1;
      if (logArg > 0) {
        return fLambda / uDotA * log(logArg);
      } else {
        return std::numeric_limits<typename decltype(vGrammage)::value_type>::infinity() *
               meter;
      }
    }
  }

} // namespace corsika
