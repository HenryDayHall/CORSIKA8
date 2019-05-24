/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_IMediumModel_h
#define _include_IMediumModel_h

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  class IMediumModel {
  public:
    virtual ~IMediumModel() = default;

    virtual corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const&) const = 0;

    // todo: think about the mixin inheritance of the trajectory vs the BaseTrajectory
    // approach for now, only lines are supported
    virtual corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const&,
        corsika::units::si::LengthType) const = 0;

    virtual corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const&,
        corsika::units::si::GrammageType) const = 0;

    virtual NuclearComposition const& GetNuclearComposition() const = 0;
  };

} // namespace corsika::environment

#endif
