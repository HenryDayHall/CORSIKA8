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
  units::si::GrammageType BaseExponential<TDerived>::integratedGrammage(
      Trajectory<Line> const& line, units::si::LengthType vL,
      Vector<units::si::dimensionless_d> const& axis) const {
    if (vL == units::si::LengthType::zero()) { return units::si::GrammageType::zero(); }

    auto const uDotA = line.NormalizedDirection().dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(line.GetR0());

    if (uDotA == 0) {
      return vL * rhoStart;
    } else {
      return rhoStart * (lambda_ / uDotA) * (exp(uDotA * vL * invLambda_) - 1);
    }
  }

  template <typename TDerived>
  units::si::LengthType BaseExponential<TDerived>::arclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType grammage,
      Vector<units::si::dimensionless_d> const& axis) const {
    auto const uDotA = line.NormalizedDirection().dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(line.GetR0());

    if (uDotA == 0) {
      return grammage / rhoStart;
    } else {
      auto const logArg = grammage * invLambda_ * uDotA / rhoStart + 1;
      if (logArg > 0) {
        return lambda_ / uDotA * log(logArg);
      } else {
        return std::numeric_limits<typename decltype(grammage)::value_type>::infinity() *
               units::si::meter;
      }
    }
  }

  template <typename TDerived>
  BaseExponential<TDerived>::BaseExponential(Point const& point,
                                             units::si::MassDensityType rho0,
                                             units::si::LengthType lambda)
      : rho0_(rho0)
      , lambda_(lambda)
      , invLambda_(1 / lambda)
      , point_(point) {}

} // namespace corsika
