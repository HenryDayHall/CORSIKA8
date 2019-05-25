/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_environment_InhomogeneousMedium_h_
#define _include_environment_InhomogeneousMedium_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

/**
 * A general inhomogeneous medium. The mass density distribution TDensityFunction must be
 * a \f$C^2\f$-function.
 */

namespace corsika::environment {

  template <class T, class TDensityFunction>
  class InhomogeneousMedium : public T {
    NuclearComposition const fNuclComp;
    TDensityFunction const fDensityFunction;

  public:
    template <typename... Args>
    InhomogeneousMedium(NuclearComposition pNuclComp, Args&&... rhoArgs)
        : fNuclComp(pNuclComp)
        , fDensityFunction(rhoArgs...) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const& p) const override {
      return fDensityFunction.EvaluateAt(p);
    }
    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& pLine,
        corsika::units::si::LengthType pTo) const override {
      return fDensityFunction.IntegrateGrammage(pLine, pTo);
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& pLine,
        corsika::units::si::GrammageType pGrammage) const override {
      return fDensityFunction.ArclengthFromGrammage(pLine, pGrammage);
    }
  };

} // namespace corsika::environment
#endif
