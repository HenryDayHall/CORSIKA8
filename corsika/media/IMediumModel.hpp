/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  class IMediumModel {

  public:
    virtual ~IMediumModel() = default; // LCOV_EXCL_LINE

    virtual MassDensityType GetMassDensity(Point const&) const = 0;

    // todo: think about the mixin inheritance of the trajectory vs the BaseTrajectory
    // approach; for now, only lines are supported
    virtual GrammageType IntegratedGrammage(Trajectory<Line> const&,
                                            LengthType) const = 0;

    virtual LengthType ArclengthFromGrammage(Trajectory<Line> const&,
                                             GrammageType) const = 0;

    virtual NuclearComposition const& GetNuclearComposition() const = 0;
  };

} // namespace corsika
