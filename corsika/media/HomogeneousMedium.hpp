/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupTrajectory.h>

#include <cassert>

/**
 * a homogeneous medium
 */

namespace corsika {

  template <class T>
  class HomogeneousMedium : public T {
    corsika::units::si::MassDensityType const fDensity;
    NuclearComposition const fNuclComp;

  public:

    HomogeneousMedium(corsika::units::si::MassDensityType pDensity, NuclearComposition pNuclComp):
        fDensity(pDensity),
        fNuclComp(pNuclComp)
  {}

    corsika::units::si::MassDensityType
	GetMassDensity( corsika::Point const&) const override
    {
      return fDensity;
    }

    NuclearComposition const&
	GetNuclearComposition() const override
    {
    	return fNuclComp;
    }

    corsika::units::si::GrammageType
	IntegratedGrammage( corsika::Trajectory<corsika::Line> const&,
        corsika::units::si::LengthType pTo) const override
    {
      using namespace corsika::units::si;
      return pTo * fDensity;
    }

    corsika::units::si::LengthType
	ArclengthFromGrammage(corsika::Trajectory<corsika::Line> const&,
        corsika::units::si::GrammageType pGrammage) const override
    {
      return pGrammage / fDensity;
    }
  };

} // namespace corsika::environment

