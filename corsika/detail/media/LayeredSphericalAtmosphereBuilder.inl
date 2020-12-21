/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/logging/Logging.hpp>

#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>

namespace corsika {

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  void LayeredSphericalAtmosphereBuilder<TMediumInterface, TMediumModelExtra,
                                         TModelArgs...>::checkRadius(LengthType r) const {
    if (r <= previousRadius_) {
      throw std::runtime_error("radius must be greater than previous");
    }
  }

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  void LayeredSphericalAtmosphereBuilder<
      TMediumInterface, TMediumModelExtra,
      TModelArgs...>::setNuclearComposition(NuclearComposition const& composition) {
    composition_ = std::make_unique<NuclearComposition>(composition);
  }

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  void LayeredSphericalAtmosphereBuilder<
      TMediumInterface, TMediumModelExtra,
      TModelArgs...>::addExponentialLayer(GrammageType b, LengthType c,
                                          LengthType upperBoundary) {

    auto const radius = earthRadius_ + upperBoundary;
    checkRadius(radius);
    previousRadius_ = radius;

    auto node = std::make_unique<VolumeTreeNode<TMediumInterface>>(
        std::make_unique<Sphere>(center_, radius));

    auto const rho0 = b / c;

    if constexpr (detail::has_extra_models<TMediumModelExtra>::value) {
      // helper lambda in which the last 5 arguments to make_shared<...> are bound
      auto lastBound = [&](auto... argPack) {
        return std::make_shared<
            TMediumModelExtra<SlidingPlanarExponential<TMediumInterface>>>(
            argPack..., center_, rho0, -c, *composition_, earthRadius_);
      };

      // now unpack the additional arguments
      auto model = std::apply(lastBound, additionalModelArgs_);
      node->setModelProperties(std::move(model));
    } else {
      node->template setModelProperties<SlidingPlanarExponential<TMediumInterface>>(
          center_, rho0, -c, *composition_, earthRadius_);
    }

    layers_.push(std::move(node));
  }

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  void LayeredSphericalAtmosphereBuilder<
      TMediumInterface, TMediumModelExtra,
      TModelArgs...>::addLinearLayer(LengthType c, LengthType upperBoundary) {
    auto const radius = earthRadius_ + upperBoundary;
    checkRadius(radius);
    previousRadius_ = radius;

    auto node = std::make_unique<VolumeTreeNode<TMediumInterface>>(
        std::make_unique<Sphere>(center_, radius));

    units::si::GrammageType constexpr b = 1 * 1_g / (1_cm * 1_cm);
    auto const rho0 = b / c;

    if constexpr (detail::has_extra_models<TMediumModelExtra>::value) {
      // helper lambda in which the last 2 arguments to make_shared<...> are bound
      auto lastBound = [&](auto... argPack) {
        return std::make_shared<TMediumModelExtra<HomogeneousMedium<TMediumInterface>>>(
            argPack..., rho0, *composition_);
      };

      // now unpack the additional arguments
      auto model = std::apply(lastBound, additionalModelArgs_);

      node->setModelProperties(std::move(model));
    } else {
      node->template setModelProperties<HomogeneousMedium<TMediumInterface>>(
          rho0, *composition_);
    }

    layers_.push(std::move(node));
  }

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  Environment<TMediumInterface> LayeredSphericalAtmosphereBuilder<
      TMediumInterface, TMediumModelExtra, TModelArgs...>::assemble() {
    Environment<TMediumInterface> env;
    assemble(env);
    return env;
  }

  template <typename TMediumInterface, template <typename> typename TMediumModelExtra,
            typename... TModelArgs>
  void LayeredSphericalAtmosphereBuilder<
      TMediumInterface, TMediumModelExtra,
      TModelArgs...>::assemble(Environment<TMediumInterface>& env) {
    auto& universe = env.getUniverse();
    auto* outmost = universe.get();

    while (!layers_.empty()) {
      auto l = std::move(layers_.top());
      auto* tmp = l.get();
      outmost->addChild(std::move(l));
      layers_.pop();
      outmost = tmp;
    }
  }

  template <typename TMediumInterface, template <typename> typename MExtraEnvirnoment>
  struct make_layered_spherical_atmosphere_builder {
    template <typename... TArgs>
    static auto create(Point const& center, LengthType earthRadius, TArgs... args) {
      return LayeredSphericalAtmosphereBuilder<TMediumInterface, MExtraEnvirnoment,
                                               TArgs...>{std::forward<TArgs>(args)...,
                                                         center, earthRadius};
    }
  };

} // namespace corsika
