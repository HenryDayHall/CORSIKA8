/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_BaseExponential_h_
#define _include_Environment_BaseExponential_h_

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <cassert>
#include <limits>

/**
 *
 */

namespace corsika::environment {

  template <class TDerived>
  class BaseExponential {
  protected:
    corsika::units::si::MassDensityType const fRho0;
    units::si::LengthType const fLambda;
    units::si::InverseLengthType const fInvLambda;
    geometry::Point const fP0;

    auto const& GetImplementation() const { return *static_cast<TDerived const*>(this); }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType pTo,
        geometry::Vector<units::si::dimensionless_d> const& axis) const {
      auto const vDotA = line.NormalizedDirection().dot(axis).magnitude();

      if (vDotA == 0) {
        return pTo * GetImplementation().GetMassDensity(line.GetR0());
      } else {
        return GetImplementation().GetMassDensity(line.GetR0()) * (fLambda / vDotA) *
               (exp(vDotA * pTo / fLambda) - 1);
      }
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType pGrammage,
        geometry::Vector<units::si::dimensionless_d> const& axis) const {
      auto const vDotA = line.NormalizedDirection().dot(axis).magnitude();

      if (vDotA == 0) {
        return pGrammage / GetImplementation().GetMassDensity(line.GetR0());
      } else {
        auto const logArg = pGrammage * vDotA / (fRho0 * fLambda) + 1;
        if (logArg > 0) {
          return fLambda / vDotA * log(pGrammage * vDotA / (fRho0 * fLambda) + 1);
        } else {
          return std::numeric_limits<typename decltype(
                     pGrammage)::value_type>::infinity() *
                 corsika::units::si::meter;
        }
      }
    }

  public:
    BaseExponential(geometry::Point const& p0, units::si::MassDensityType rho,
                    units::si::LengthType lambda)
        : fRho0(rho)
        , fLambda(lambda)
        , fInvLambda(1 / lambda)
        , fP0(p0) {}
  };

} // namespace corsika::environment
#endif
