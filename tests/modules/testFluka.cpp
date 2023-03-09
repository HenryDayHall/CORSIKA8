/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/FLUKA.hpp>
#include <corsika/modules/fluka/Random.hpp>
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

#include <SetupTestStack.hpp>
#include <SetupTestEnvironment.hpp>

using namespace corsika;

template <typename TStackView>
auto sumMomentum(TStackView const& view, CoordinateSystemPtr const& vCS) {
  MomentumVector sum{vCS, 0_eV, 0_eV, 0_eV};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("FLUKACodeConversion") {
  REQUIRE(corsika::fluka::convertToFluka(Code::PiPlus) ==
          corsika::fluka::FLUKACode::PiPlus);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::PiPlus) == 13);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::Proton) == 1);
  REQUIRE(corsika::fluka::convertToFlukaRaw(Code::Lambda0) == 17);
}

TEST_CASE("FLUKA") {
  using DummyEnvironmentInterface =
      IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
  using DummyEnvironment = Environment<DummyEnvironmentInterface>;
  using MyHomogeneousModel = MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<DummyEnvironmentInterface>>>;

  RNGManager<>::getInstance().registerRandomStream("fluka");

  DummyEnvironment env;
  auto& universe = *env.getUniverse();
  CoordinateSystemPtr const& cs = env.getCoordinateSystem();
  universe.setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, Vector(cs, 0_T, 0_T, 0_T), 1_kg / (1_m * 1_m * 1_m),
      NuclearComposition{
          std::vector<Code>{Code::Hydrogen, Code::Oxygen, Code::Nitrogen, Code::Argon},
          std::vector<double>{.25, .25, .25, .25}});

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
    for (auto const& [projectileCode, targetCode] : combinationsOK) {
      auto const projectile4mom =
          FourVector{calculate_total_energy(p, get_mass(projectileCode)),
                     MomentumVector{cs, 0_eV, 0_eV, p}};
      auto const target4mom =
          FourVector{get_mass(targetCode), MomentumVector{cs, 0_eV, 0_eV, 0_eV}};

      CHECK(flukaModel.getCrossSection(projectileCode, targetCode, projectile4mom,
                                       target4mom) > 0_mb);
    }
  }

  {
    auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Proton);
    auto const& cs = *csPtr;

    auto const projectiles = std::array{Code::PiPlus, Code::PiMinus, Code::KPlus,
                                        Code::K0Long, Code::Lambda0, Code::SigmaPlus};
    auto const momenta = std::array{100_MeV, 1_GeV, 20_GeV, 100_GeV, 1_TeV};

    for (auto const p : momenta) {
      for (auto const projectileCode : projectiles) {
        std::cout << "===== " << projectileCode << " @ " << p << "\n";

        auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
            Code::Hydrogen, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr,
            *csPtr);
        { [[maybe_unused]] auto const& dummy_StackPtr = stackPtr; }

        auto const targetCode = Code::Oxygen;
        auto const projectile4mom =
            FourVector{calculate_total_energy(p, get_mass(projectileCode)),
                       MomentumVector{cs, 0_eV, 0_eV, p}};
        auto const target4mom =
            FourVector{get_mass(targetCode), MomentumVector{cs, 0_eV, 0_eV, 0_eV}};

        flukaModel.doInteraction(*secViewPtr, projectileCode, targetCode, projectile4mom,
                                 target4mom);
        auto const pSum = sumMomentum(*secViewPtr, cs);
        std::cout << "psum = " << pSum << '\n';
        CHECK((pSum - projectile4mom.getSpaceLikeComponents()).getNorm() / p ==
              Approx(0).margin(1e-4));
        CHECK((pSum.getNorm() - p) / p == Approx(0).margin(1e-4));
      }
    }
  }
}
