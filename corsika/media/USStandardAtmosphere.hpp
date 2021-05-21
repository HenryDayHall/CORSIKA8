/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/IRefractiveIndexModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/framework/utility/ImplementsMixin.hpp>

namespace corsika {

  template <typename TEnvironmentInterface, template <typename> typename TExtraEnv,
            typename TEnvironment, typename... TArgs>
  auto create_us_standard_atmosphere(TEnvironment& env, Point const& center,
                                     LengthType const& earthRadius, TArgs... args) {

    // construct the atmosphere builder
    auto builder = make_layered_spherical_atmosphere_builder<
        TEnvironmentInterface, TExtraEnv>::create(center, earthRadius,
                                                  std::forward<TArgs>(args)...);

    // as per the vertical_EAS reference, we do not include Ar for now.
    // TODO: This is not a US standard atmosphere
    builder.setNuclearComposition(
        {{Code::Nitrogen, Code::Oxygen}, {0.7847f, 1.f - 0.7847f}});

    // add the standard atmosphere layers
    builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 2_km);
    builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
    builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
    builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
    builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
    builder.addLinearLayer(1e9_cm, 112.8_km + constants::EarthRadius::Mean);

    // check if we want to also add the US standard refractivity
    if constexpr (detail::implements_mixin<IRefractiveIndexModel,
                                           TEnvironmentInterface>::value) {

      // TODO: Add US Standard refractivity
    }

    // and assemble the environment
    builder.assemble(env);
  };

} // namespace corsika
