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
#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  template <typename TDerived>
  inline auto const& BaseExponential<TDerived>::getImplementation() const {
    return *static_cast<TDerived const*>(this);
  }

  template <typename TDerived>
  inline GrammageType BaseExponential<TDerived>::getIntegratedGrammage(
      BaseTrajectory const& traj, LengthType vL, DirectionVector const& axis) const {
    if (vL == LengthType::zero()) { return GrammageType::zero(); }

    auto const uDotA = traj.getDirection(0).dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(traj.getPosition(0));

    if (uDotA == 0) {
      return vL * rhoStart;
    } else {
      return rhoStart * (lambda_ / uDotA) * (exp(uDotA * vL * invLambda_) - 1);
    }
  }

  template <typename TDerived>
  inline LengthType BaseExponential<TDerived>::getArclengthFromGrammage(
      BaseTrajectory const& traj, GrammageType grammage,
      DirectionVector const& axis) const {
    auto const uDotA = traj.getDirection(0).dot(axis).magnitude();
    auto const rhoStart = getImplementation().getMassDensity(traj.getPosition(0));

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
  inline BaseExponential<TDerived>::BaseExponential(Point const& point,
                                                    MassDensityType rho0,
                                                    LengthType lambda)
      : rho0_(rho0)
      , lambda_(lambda)
      , invLambda_(1 / lambda)
      , point_(point) {}

} // namespace corsika
