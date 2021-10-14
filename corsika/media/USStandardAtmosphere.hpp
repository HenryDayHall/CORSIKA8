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

  /**
   * Atmosphere Ids following the CORSIKA 7 5-layered atmosphere models.
   *
   * Each model corresponds to a standard 5-layered atmosphere model. 4 Layers are
   * exponential, the outer layer is with constant density.
   *
   * All atmospheres are valid for heights (above Earth sea level) up to 112.8 km.
   */
  enum class AtmosphereId : uint8_t {
    LinsleyUSStd = 0,
    /*    MiddleEuropeJan,
        MiddleEuropeFeb,
        MiddleEuropeMay,
        MiddleEuropeJun,
        MiddleEuropeAug,
        MiddleEuropeOct,
        MiddleEuropeDec,
        SouthPoleMar,
        SouthPoleJul,
        SouthPoleOct,
        SouthPoleDec,
        SouthPoleJan,
        SouthPoleAug,
        MalargueWinterI,
        MalargueWinterII,
        MalargueSpring,
        MalargueSummer,
        MalargueAutumn,
        USStdBK,*/
    LastAtmosphere
  };

  struct AtmosphereLayerParameters {
    LengthType altitude;
    GrammageType offset;
    LengthType scaleHeight;
  };

  typedef std::array<AtmosphereLayerParameters, 5> AtmosphereParameters;

  std::arrary<AtmosphereParameters,
              static_cast<uint8_t>(
                  AtmosphereId::LastAtmosphere)> constexpr AtmosphereParameterList{
      {{4_km, 1222.6562_g / cube(1_cm), 994186.38_cm},
       {10_km, 1144.9069_g / cube(1_cm), 878153.55_cm},
       {40_km, 1305.5948_g / cube(1_cm), 636143.04_cm},
       {100_km, 540.1778_g / cube(1_cm), 772170.16_cm, 100_km},
       {112.8_km, 1_g / cube(1_cm), 1e9_cm}}};

  template <typename TEnvironmentInterface, template <typename> typename TExtraEnv,
            typename TEnvironment, typename... TArgs>
  auto create_5layer_atmosphere(AtmosphereId const atmId, TEnvironment& env,
                                Point const& center, TArgs... args) {

    // construct the atmosphere builder
    auto builder = make_layered_spherical_atmosphere_builder<
        TEnvironmentInterface, TExtraEnv>::create(center, constants::EarthRadius::Mean,
                                                  std::forward<TArgs>(args)...);

    // as per the vertical_EAS reference, we do not include Ar for now.
    // TODO: This is not a US standard atmosphere
    builder.setNuclearComposition(
        {{Code::Nitrogen, Code::Oxygen}, {0.7847, 1. - 0.7847}});

    // add the standard atmosphere layers
    auto params = AtmosphereParameterList[static_cast<uint8_t>(atmId)];
    for (int i = 0; i < 4; ++i) {
      builder.addExponentialLayer(params[i][1], params[i][2], params[i][0]);
    }
    builder.addLinearLayer(params[4][2], params[4][0]);
    /*
      builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
    builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
    builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
    builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
    builder.addLinearLayer(1e9_cm, 112.8_km);*/

    // check if we want to also add the US standard refractivity
    if constexpr (detail::implements_mixin_v<IRefractiveIndexModel,
                                             TEnvironmentInterface>) {

      // TODO: Add US Standard refractivity
    }

    // and assemble the environment
    builder.assemble(env);
  };

} // namespace corsika
