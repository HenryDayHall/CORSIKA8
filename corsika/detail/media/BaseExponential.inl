/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

namespace corsika {

  template <typename TDerived>
  auto const& BaseExponential<TDerived>::getImplementation() const {
    return *static_cast<TDerived const*>(this);
  }

  template <typename TDerived>
  GrammageType BaseExponential<TDerived>::getIntegratedGrammage(
      Trajectory<Line> const& line, LengthType vL,
      Vector<dimensionless_d> const& axis) const {
    if (vL == LengthType::zero()) { return GrammageType::zero(); }

    auto const uDotA = line.getNormalizedDirection().dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(line.getStartPoint());

    if (uDotA == 0) {
      return vL * rhoStart;
    } else {
      return rhoStart * (lambda_ / uDotA) * (exp(uDotA * vL * invLambda_) - 1);
    }
  }

  template <typename TDerived>
  LengthType BaseExponential<TDerived>::getArclengthFromGrammage(
      Trajectory<Line> const& line, GrammageType grammage,
      Vector<dimensionless_d> const& axis) const {
    auto const uDotA = line.getNormalizedDirection().dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(line.getStartPoint());

    if (uDotA == 0) {
      return grammage / rhoStart;
    } else {
      auto const logArg = grammage * invLambda_ * uDotA / rhoStart + 1;
      if (logArg > 0) {
        return lambda_ / uDotA * log(logArg);
      } else {
        return std::numeric_limits<typename decltype(grammage)::value_type>::infinity() *
               meter;
      }
    }
  }

  template <typename TDerived>
  BaseExponential<TDerived>::BaseExponential(Point const& point, MassDensityType rho0,
                                             LengthType lambda)
      : rho0_(rho0)
      , lambda_(lambda)
      , invLambda_(1 / lambda)
      , point_(point) {}

} // namespace corsika
