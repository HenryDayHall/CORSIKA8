/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_SlidingPlanarExponential_h_
#define _include_Environment_SlidingPlanarExponential_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <cassert>
#include <limits>

/**
 *
 */

namespace corsika::environment {

  template <class T>
  class SlidingPlanarExponential : public T {
    corsika::units::si::MassDensityType const fRho0;
    units::si::LengthType const fLambda;
    units::si::InverseLengthType const fInvLambda;
    NuclearComposition const fNuclComp;
    geometry::Vector<units::si::dimensionless_d> const fAxis;
    geometry::Point const fP0;

  public:
    SlidingPlanarExponential(geometry::Point const& p0,
                    geometry::Vector<units::si::dimensionless_d> const& axis,
                    units::si::MassDensityType rho, units::si::LengthType lambda,
                    NuclearComposition pNuclComp)
        : fRho0(rho)
        , fLambda(lambda)
        , fInvLambda(1 / lambda)
        , fNuclComp(pNuclComp)
        , fAxis(axis)
        , fP0(p0) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const& p) const override {
      auto const height = (p - fP0).norm();
      return fRho0 * exp(fInvLambda * height);
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType pTo) const override {

      auto const axis = (fP0 - line.GetR0()).normalized();
      auto const vDotA = line.NormalizedDirection().dot(fAxis).magnitude();

      if (vDotA == 0) {
        return pTo * GetMassDensity(line.GetR0());
      } else {
        return GetMassDensity(line.GetR0()) * (fLambda / vDotA) *
               (exp(vDotA * pTo / fLambda) - 1);
      }
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType pGrammage) const override {
      auto const axis = (fP0 - line.GetR0()).normalized();
      auto const vDotA = line.NormalizedDirection().dot(fAxis).magnitude();

      if (vDotA == 0) {
        return pGrammage / GetMassDensity(line.GetR0());
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
  };

} // namespace corsika::environment
#endif
