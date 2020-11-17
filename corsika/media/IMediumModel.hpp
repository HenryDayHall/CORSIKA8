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
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  class IMediumModel {
  public:
    virtual ~IMediumModel() = default; // LCOV_EXCL_LINE

    virtual units::si::MassDensityType getMassDensity(Point const&) const = 0;

    // todo: think about the mixin inheritance of the trajectory vs the BaseTrajectory
    // approach; for now, only lines are supported
    virtual units::si::GrammageType integratedGrammage(
        Trajectory<Line> const&, units::si::LengthType) const = 0;

    virtual units::si::LengthType arclengthFromGrammage(
        Trajectory<Line> const&, units::si::GrammageType) const = 0;

    virtual NuclearComposition const& getNuclearComposition() const = 0;
  };

} // namespace corsika
