/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/SlidingPlanarExponential.h>
#include <corsika/environment/VolumeTreeNode.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>

#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>

namespace corsika::environment {

  // forward-decl
  template <typename TMediumInterface, template <typename> typename MExtraEnvirnoment>
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

  namespace detail {

    struct NoExtraModelInner {};

    template <typename M>
    struct NoExtraModel {};

    template <template <typename> typename M>
    struct has_extra_models : std::true_type {};

    template <>
    struct has_extra_models<NoExtraModel> : std::false_type {};

  } // namespace detail

  template <typename TMediumInterface = environment::IMediumModel,
            template <typename> typename TMediumModelExtra = detail::NoExtraModel,
            typename... TModelArgs>
  class LayeredSphericalAtmosphereBuilder {
    std::unique_ptr<NuclearComposition> composition_;
    geometry::Point center_;
    units::si::LengthType previousRadius_{units::si::LengthType::zero()};
    units::si::LengthType earthRadius_;
    std::tuple<TModelArgs...> const additionalModelArgs_;

    std::stack<typename VolumeTreeNode<TMediumInterface>::VTNUPtr>
        layers_; // innermost layer first

    void checkRadius(units::si::LengthType r) const {
      if (r <= previousRadius_) {
        throw std::runtime_error("radius must be greater than previous");
      }
    }

    LayeredSphericalAtmosphereBuilder() = delete;
    LayeredSphericalAtmosphereBuilder(const LayeredSphericalAtmosphereBuilder&) = delete;
    LayeredSphericalAtmosphereBuilder(const LayeredSphericalAtmosphereBuilder&&) = delete;
    LayeredSphericalAtmosphereBuilder& operator=(
        const LayeredSphericalAtmosphereBuilder&) = delete;

    // friend, to allow construction
    template <typename, template <typename> typename>
    friend struct make_layered_spherical_atmosphere_builder;

  protected:
    LayeredSphericalAtmosphereBuilder(TModelArgs... args, corsika::geometry::Point center,
                                      units::si::LengthType earthRadius)
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
  template <typename TMediumInterface = environment::IMediumModel,
            template <typename> typename MExtraEnvirnoment = detail::NoExtraModel>
  struct make_layered_spherical_atmosphere_builder {
    template <typename... TArgs>
    static auto create(geometry::Point const& center, units::si::LengthType earthRadius,
                       TArgs... args) {
      return environment::LayeredSphericalAtmosphereBuilder<TMediumInterface,
                                                            MExtraEnvirnoment, TArgs...>{
          std::forward<TArgs>(args)..., center, earthRadius};
    }
  };

} // namespace corsika::environment
