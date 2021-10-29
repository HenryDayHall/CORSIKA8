/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  template <typename TEnvironmentInterface, template <typename> typename TExtraEnv,
            typename TEnvironment, typename... TArgs>
  void create_5layer_atmosphere(TEnvironment& env, AtmosphereId const atmId,
                                Point const& center, TArgs... args) {

    // construct the atmosphere builder
    auto builder = make_layered_spherical_atmosphere_builder<
        TEnvironmentInterface, TExtraEnv>::create(center, constants::EarthRadius::Mean,
                                                  std::forward<TArgs>(args)...);

    // composition values from AIRES manual
    builder.setNuclearComposition({{
                                       Code::Nitrogen,
                                       Code::Argon,
                                       Code::Oxygen,
                                   },
                                   {0.7847, 0.0047, 1. - 0.7847 - 0.0047}});

    // add the standard atmosphere layers
    auto const params = atmosphereParameterList[static_cast<uint8_t>(atmId)];
    for (int i = 0; i < 4; ++i) {
      builder.addExponentialLayer(params[i].offset, params[i].scaleHeight,
                                  params[i].altitude);
    }
    builder.addLinearLayer(params[4].offset, params[4].scaleHeight, params[4].altitude);

    // and assemble the environment
    builder.assemble(env);
  }

} // namespace corsika
