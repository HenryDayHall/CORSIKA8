/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/VolumeTreeNode.hpp>

#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>

namespace corsika {

  class LayeredSphericalAtmosphereBuilder {
    std::unique_ptr<NuclearComposition> composition_;
    Point center_;
    LengthType previousRadius_{LengthType::zero()};
    LengthType seaLevel_;

    std::stack<VolumeTreeNode<IMediumModel>::VTNUPtr> layers_; // innermost layer first

    void checkRadius(LengthType) const;

  public:
    static auto constexpr earthRadius = 6'371'000 * meter;

    LayeredSphericalAtmosphereBuilder(corsika::Point center,
                                      LengthType seaLevel = earthRadius)
        : center_(center)
        , earthRadius_(earthRadius)
        , additionalModelArgs_{args...} {}

    void setNuclearComposition(NuclearComposition);

    void addExponentialLayer(GrammageType, LengthType, LengthType);

    auto size() const { return layers_.size(); }

    void addLinearLayer(LengthType, LengthType);

    void assemble(Environment<IMediumModel>&);

    Environment<IMediumModel> assemble();
  };

} // namespace corsika

#include <corsika/detail/media/LayeredSphericalAtmosphereBuilder.inl>
