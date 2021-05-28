/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/BaseTrajectory.hpp>

namespace corsika {

  class IMediumModel {
  public:
    virtual ~IMediumModel() = default; // LCOV_EXCL_LINE

    virtual MassDensityType getMassDensity(Point const&) const = 0;

    // todo: think about the mixin inheritance of the trajectory vs the BaseTrajectory
    // approach; for now, only lines are supported
    virtual GrammageType getIntegratedGrammage(BaseTrajectory const&,
                                               LengthType) const = 0;

    virtual LengthType getArclengthFromGrammage(BaseTrajectory const&,
                                                GrammageType) const = 0;

    virtual NuclearComposition const& getNuclearComposition() const = 0;
  };

} // namespace corsika
