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

// for detail namespace, NoExtraModelInner, NoExtraModel and traits
#include <corsika/detail/media/LayeredSphericalAtmosphereBuilder.hpp>

#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>

namespace corsika {

  /**
   * \class make_layered_spherical_atmosphere_builder
   *
   * Helper class to create LayeredSphericalAtmosphereBuilder, the
   * extra environment models have to be passed as template-template
   * argument to make_layered_spherical_atmosphere_builder, the member
   * function `create` does then take an unspecified number of extra
   * parameters to internalize those models for all layers later
   * produced.
   **/
  template <typename TMediumInterface = IMediumModel,
            template <typename> typename MExtraEnvirnoment = detail::NoExtraModel>
  struct make_layered_spherical_atmosphere_builder;

  /**
   * Helper class to setup concentric spheres of layered atmosphere
   * with spcified density profiles (exponential, linear, ...).
   *
   * This can be used most importantly to replicate CORSIKA7
   * atmospheres.
   *
   * Each layer by definition has a density profile and a (constant)
   * nuclear composition model.
   *
   */

  template <typename TMediumInterface = IMediumModel,
            template <typename> typename TMediumModelExtra = detail::NoExtraModel,
            typename... TModelArgs>
  class LayeredSphericalAtmosphereBuilder {

    LayeredSphericalAtmosphereBuilder() = delete;
    LayeredSphericalAtmosphereBuilder(const LayeredSphericalAtmosphereBuilder&) = delete;
    LayeredSphericalAtmosphereBuilder(const LayeredSphericalAtmosphereBuilder&&) = delete;
    LayeredSphericalAtmosphereBuilder& operator=(
        const LayeredSphericalAtmosphereBuilder&) = delete;

    // friend, to allow construction
    template <typename, template <typename> typename>
    friend struct make_layered_spherical_atmosphere_builder;

  protected:
    LayeredSphericalAtmosphereBuilder(TModelArgs... args, Point const& center,
                                      LengthType earthRadius)
        : center_(center)
        , earthRadius_(earthRadius)
        , additionalModelArgs_{args...} {}

  public:
    void setNuclearComposition(NuclearComposition const& composition);
    void addExponentialLayer(GrammageType b, LengthType c, LengthType upperBoundary);
    void addLinearLayer(LengthType c, LengthType upperBoundary);

    int getSize() const { return layers_.size(); }

    void assemble(Environment<TMediumInterface>& env);
    Environment<TMediumInterface> assemble();

    /**
     * Get the current Earth radius.
     */
    LengthType getEarthRadius() const { return earthRadius_; }

  private:
    void checkRadius(LengthType r) const;

    std::unique_ptr<NuclearComposition> composition_;
    Point center_;
    LengthType previousRadius_{LengthType::zero()};
    LengthType earthRadius_;
    std::tuple<TModelArgs...> const additionalModelArgs_;

    std::stack<typename VolumeTreeNode<TMediumInterface>::VTNUPtr>
        layers_; // innermost layer first
    
  }; // end class LayeredSphericalAtmosphereBuilder

} // namespace corsika

#include <corsika/detail/media/LayeredSphericalAtmosphereBuilder.inl>
