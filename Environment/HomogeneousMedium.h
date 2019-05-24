/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_HomogeneousMedium_h_
#define _include_HomogeneousMedium_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <cassert>

/**
 * a homogeneous medium
 */

namespace corsika::environment {

  template <class T>
  class HomogeneousMedium : public T {
    corsika::units::si::MassDensityType const fDensity;
    NuclearComposition const fNuclComp;

  public:
    HomogeneousMedium(corsika::units::si::MassDensityType pDensity,
                      NuclearComposition pNuclComp)
        : fDensity(pDensity)
        , fNuclComp(pNuclComp) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const&) const override {
      return fDensity;
    }
    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const&,
        corsika::units::si::LengthType pTo) const override {
      using namespace corsika::units::si;
      return pTo * fDensity;
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const&,
        corsika::units::si::GrammageType pGrammage) const override {
      return pGrammage / fDensity;
    }
  };

} // namespace corsika::environment
#endif
