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
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/media/NuclearComposition.hpp>

/**
 * A general inhomogeneous medium. The mass density distribution TDensityFunction must be
 * a \f$C^2\f$-function.
 */

namespace corsika {

  template <class T, class TDensityFunction>
  class InhomogeneousMedium : public T {
    NuclearComposition const fNuclComp;
    TDensityFunction const fDensityFunction;

  public:
    /*
     * FIXME: should have traits here for ...Args
     */
    template <typename... Args>
    InhomogeneousMedium(NuclearComposition pNuclComp, Args&&... rhoArgs)
        : fNuclComp(pNuclComp)
        , fDensityFunction(rhoArgs...) {}

    MassDensityType GetMassDensity(Point const& p) const override {
      return fDensityFunction.EvaluateAt(p);
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    GrammageType IntegratedGrammage(Trajectory<Line> const& pLine,
                                    LengthType pTo) const override {
      return fDensityFunction.IntegrateGrammage(pLine, pTo);
    }

    LengthType ArclengthFromGrammage(Trajectory<Line> const& pLine,
                                     GrammageType pGrammage) const override {
      return fDensityFunction.ArclengthFromGrammage(pLine, pGrammage);
    }
  };

} // namespace corsika
