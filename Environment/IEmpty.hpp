/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/units/PhysicalUnits.h>
#include <corsika/geometry/Trajectory.h>

namespace corsika::environment {

  /**
   * \class IEmpty
   *
   * intended for usage as default template argument for environments
   * with no properties.  For now, the ArclengthFromGrammage is
   * mandatory, since it is used even in the most simple Cascade code.
   *
   * - IEmpty is the interface definition.
   * - Empty<IEmpty> is a possible model implementation
   *
   **/

  class IEmpty {
  public:
    virtual corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::LineTrajectory const&,
        corsika::units::si::GrammageType) const = 0;
  };

  template <typename TModel = IEmpty>
  class Empty : public TModel {
  public:
    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::LineTrajectory const&,
        corsika::units::si::GrammageType) const {
      return 0. * corsika::units::si::meter;
    }
  };

} // namespace corsika::environment
