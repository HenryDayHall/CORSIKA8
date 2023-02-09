/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/FLUKA.hpp>
//~ #include <SetupTestEnvironment.hpp>

#include <corsika/framework/core/EnergyMomentumOperations.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/HomogeneousMedium.hpp>

#include <catch2/catch.hpp>

#include <fstream>
#include <iomanip>

using namespace corsika;

TEST_CASE("FLUKACodeConversion") {
  REQUIRE(corsika::fluka::convertToFluka(Code::PiPlus) ==
          corsika::fluka::FLUKACode::PiPlus);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::PiPlus) == 13);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::Proton) == 1);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::Lambda0) == 17);
}

TEST_CASE("FLUKA") {
  using DummyEnvironmentInterface = IMediumModel;
  using DummyEnvironment = Environment<DummyEnvironmentInterface>;
  using MyHomogeneousModel = HomogeneousMedium<DummyEnvironmentInterface>;

  DummyEnvironment env;
  auto& universe = *env.getUniverse();
  CoordinateSystemPtr const& cs = env.getCoordinateSystem();
  universe.setModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(
          std::vector<Code>{Code::Hydrogen, Code::Oxygen, Code::Nitrogen, Code::Argon},
          std::vector<double>{.25, .25, .25, .25}));

  corsika::fluka::InteractionModel flukaModel{env};

  // test getMaterialIndex
  REQUIRE(flukaModel.getMaterialIndex(Code::Hydrogen) > 0);
  REQUIRE(flukaModel.getMaterialIndex(Code::Oxygen) > 0);
  REQUIRE(flukaModel.getMaterialIndex(Code::Nitrogen) > 0);
  REQUIRE(flukaModel.getMaterialIndex(Code::Argon) > 0);
  REQUIRE(flukaModel.getMaterialIndex(Code::Uranium) < 0);

  { // test getCrossSection for allowed projectile/target combinations
    auto combinationsOK = std::vector{
        std::tuple{Code::PiMinus, Code::Hydrogen},
        std::tuple{Code::PiMinus, Code::Nitrogen},
        std::tuple{Code::PiMinus, Code::Oxygen},
        std::tuple{Code::KMinus, Code::Oxygen},
        std::tuple{Code::K0Long, Code::Oxygen},
        std::tuple{Code::K0Short, Code::Oxygen},
        std::tuple{Code::Lambda0, Code::Oxygen},
        std::tuple{Code::SigmaPlus, Code::Oxygen},
        std::tuple{Code::Proton, Code::Oxygen},
        std::tuple{Code::AntiProton, Code::Oxygen},
        std::tuple{Code::KMinus, Code::Hydrogen},
        std::tuple{Code::K0Long, Code::Hydrogen},
        std::tuple{Code::K0Short, Code::Hydrogen},
        std::tuple{Code::Lambda0, Code::Hydrogen},
        std::tuple{Code::SigmaPlus, Code::Hydrogen},
        std::tuple{Code::Proton, Code::Hydrogen},
        std::tuple{Code::AntiProton, Code::Hydrogen},
    };

    HEPEnergyType const p = 100_GeV;
    for (auto const& [projectileCode, targetCode] : combinations) {
      auto const projectile4mom =
          FourVector{calculate_total_energy(p, get_mass(projectileCode)),
                     MomentumVector{cs, 0_eV, 0_eV, p}};
      auto const target4mom =
          FourVector{get_mass(targetCode), MomentumVector{cs, 0_eV, 0_eV, 0_eV}};

      CHECK(flukaModel.getCrossSection(projectileCode, targetCode, projectile4mom,
                                       target4mom) > 0_mb);
    }
  }
}
