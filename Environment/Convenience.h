/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <memory>
#include <stack>

namespace corsika::environment {

  class LayeredSphericalAtmosphereBuilder {
    std::unique_ptr<NuclearComposition> composition_;
    geometry::Point center_;
    units::si::LengthType previousRadius_{units::si::LengthType::zero()};
    units::si::LengthType seaLevel_;

    std::stack<VolumeTreeNode<environment::IMediumModel>::VTNUPtr>
        layers_; // innermost layer first

    void checkRadius(units::si::LengthType) const;

  public:
    static auto constexpr earthRadius = 6'371'000 * units::si::meter;

    LayeredSphericalAtmosphereBuilder(corsika::geometry::Point center,
                                      units::si::LengthType seaLevel = earthRadius)
        : center_(center)
        , seaLevel_(seaLevel) {}

    void setNuclearComposition(NuclearComposition);

    void addExponentialLayer(units::si::GrammageType, units::si::LengthType,
                             units::si::LengthType);

    auto size() const { return layers_.size(); }

    void addLinearLayer(units::si::LengthType, units::si::LengthType);

    Environment<IMediumModel> assemble();
  };

} // namespace corsika::environment
