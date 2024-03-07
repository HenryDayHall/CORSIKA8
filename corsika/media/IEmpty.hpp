/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/BaseTrajectory.hpp>

namespace corsika {

  /**
   * Intended for usage as default template argument for environments
   * with no properties.  For now, the ArclengthFromGrammage is
   * mandatory, since it is used even in the most simple Cascade code.
   *
   * - IEmpty is the interface definition.
   * - Empty<IEmpty> is a possible model implementation
   *
   **/

  class IEmpty {
  public:
    virtual LengthType getArclengthFromGrammage(BaseTrajectory const&,
                                                GrammageType) const = 0;

    virtual NuclearComposition const& getNuclearComposition() const = 0;

    virtual ~IEmpty() {}
  };

  template <typename TModel = IEmpty>
  class Empty : public TModel {
  public:
    LengthType getArclengthFromGrammage(BaseTrajectory const&, GrammageType) const {
      return 0. * meter;
    }
    NuclearComposition const& getNuclearComposition() const {
      return NuclearComposition(std::vector<Code>{}, {});
    };
  };

} // namespace corsika
