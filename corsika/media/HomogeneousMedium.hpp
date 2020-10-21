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

#include <corsika/setup/SetupTrajectory.h>

#include <cassert>

/**
 * a homogeneous medium
 */

namespace corsika {

  template <class T>
  class HomogeneousMedium : public T {
    MassDensityType const fDensity;
    NuclearComposition const fNuclComp;

  public:
    HomogeneousMedium(MassDensityType pDensity, NuclearComposition pNuclComp)
        : fDensity(pDensity)
        , fNuclComp(pNuclComp) {}

    MassDensityType GetMassDensity(corsika::Point const&) const override {
      return fDensity;
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    GrammageType IntegratedGrammage(corsika::Trajectory<corsika::Line> const&,
                                    LengthType pTo) const override {
      return pTo * fDensity;
    }

    LengthType ArclengthFromGrammage(corsika::Trajectory<corsika::Line> const&,
                                     GrammageType pGrammage) const override {
      return pGrammage / fDensity;
    }
  };

} // namespace corsika
