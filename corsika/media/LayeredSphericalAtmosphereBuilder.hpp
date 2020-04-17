/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>

namespace corsika {

  class LayeredSphericalAtmosphereBuilder
  {
    std::unique_ptr<NuclearComposition> composition_;
    Point center_;
    units::si::LengthType previousRadius_{units::si::LengthType::zero()};
    units::si::LengthType earthRadius_;
    std::tuple<TModelArgs...> const additionalModelArgs_;

    std::stack<VolumeTreeNode<IMediumModel>::VTNUPtr>
        layers_; // innermost layer first

    void checkRadius(units::si::LengthType r) const {
      if (r <= previousRadius_) {
        throw std::runtime_error("radius must be greater than previous");
      }
    }

  public:

    static auto constexpr earthRadius = 6'371'000 * units::si::meter;

    LayeredSphericalAtmosphereBuilder(corsika::Point center,
                                      units::si::LengthType seaLevel = earthRadius)
        : center_(center)
        , earthRadius_(earthRadius)
        , additionalModelArgs_{args...} {}

  public:
    void setNuclearComposition(NuclearComposition composition) {
      composition_ = std::make_unique<NuclearComposition>(composition);
    }

    void addExponentialLayer(units::si::GrammageType b, units::si::LengthType c,
                             units::si::LengthType upperBoundary) {
      using namespace units::si;

      auto const radius = earthRadius_ + upperBoundary;
      checkRadius(radius);
      previousRadius_ = radius;

      auto node = std::make_unique<VolumeTreeNode<TMediumInterface>>(
          std::make_unique<geometry::Sphere>(center_, radius));

      auto const rho0 = b / c;
      std::cout << "rho0 = " << rho0 << ", c = " << c << std::endl;

      if constexpr (detail::has_extra_models<TMediumModelExtra>::value) {
        // helper lambda in which the last 5 arguments to make_shared<...> are bound
        auto lastBound = [&](auto... argPack) {
          return std::make_shared<
              TMediumModelExtra<environment::SlidingPlanarExponential<TMediumInterface>>>(
              argPack..., center_, rho0, -c, *composition_, earthRadius_);
        };

        // now unpack the additional arguments
        auto model = std::apply(lastBound, additionalModelArgs_);
        node->SetModelProperties(std::move(model));
      } else {
        node->template SetModelProperties<SlidingPlanarExponential<TMediumInterface>>(
            center_, rho0, -c, *composition_, earthRadius_);
      }

      layers_.push(std::move(node));
    }

    void addLinearLayer(units::si::LengthType c, units::si::LengthType upperBoundary) {
      using namespace units::si;

      auto const radius = earthRadius_ + upperBoundary;
      checkRadius(radius);
      previousRadius_ = radius;

      auto node = std::make_unique<VolumeTreeNode<TMediumInterface>>(
          std::make_unique<geometry::Sphere>(center_, radius));

      units::si::GrammageType constexpr b = 1 * 1_g / (1_cm * 1_cm);
      auto const rho0 = b / c;

      std::cout << "rho0 = " << rho0;

      if constexpr (detail::has_extra_models<TMediumModelExtra>::value) {
        // helper lambda in which the last 2 arguments to make_shared<...> are bound
        auto lastBound = [&](auto... argPack) {
          return std::make_shared<
              TMediumModelExtra<environment::HomogeneousMedium<TMediumInterface>>>(
              argPack..., rho0, *composition_);
        };

        // now unpack the additional arguments
        auto model = std::apply(lastBound, additionalModelArgs_);

        node->SetModelProperties(std::move(model));
      } else {
        node->template SetModelProperties<
            environment::HomogeneousMedium<TMediumInterface>>(rho0, *composition_);
      }

      layers_.push(std::move(node));
    }

    int size() const { return layers_.size(); }

    void assemble(Environment<TMediumInterface>& env) {
      auto& universe = env.GetUniverse();
      auto* outmost = universe.get();

      while (!layers_.empty()) {
        auto l = std::move(layers_.top());
        auto* tmp = l.get();
        outmost->AddChild(std::move(l));
        layers_.pop();
        outmost = tmp;
      }
    }

    Environment<TMediumInterface> assemble() {
      Environment<TMediumInterface> env;
      assemble(env);
      return env;
    }

    /**
     * Get the current Earth radius.
     */
    units::si::LengthType getEarthRadius() const { return earthRadius_; }

  }; // end class LayeredSphericalAtmosphereBuilder

    void assemble(Environment<IMediumModel>&);

    Environment<IMediumModel> assemble();

  };

} // namespace corsika::environment

#include <corsika/detail/media/LayeredSphericalAtmosphereBuilder.inl>
