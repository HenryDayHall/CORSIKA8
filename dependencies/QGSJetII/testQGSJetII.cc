/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/qgsjetII/Interaction.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>

#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/framework/coreParticleProperties.h>

#include <corsika/framework/geometry/Point.h>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>

using namespace corsika;
using namespace corsika::qgsjetII;
using namespace corsika::units::si;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;

  for (auto const& p : view) { totalCharge += GetChargeNumber(p.GetPID()); }

  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

TEST_CASE("CORSIKA_DATA", "[processes]") {

  SECTION("check CORSIKA_DATA") {

    const char* data = std::getenv("CORSIKA_DATA");
    // these REQUIRES are needed:
    REQUIRE(data != 0);
    REQUIRE(std::experimental::filesystem::is_directory(
        std::experimental::filesystem::path(std::string(data) + "/QGSJetII")));
    std::cout << "data: " << data << " isDir: "
              << std::experimental::filesystem::is_directory(std::string(data) +
                                                             "/QGSJetII")
              << std::endl;
  }
}

TEST_CASE("QgsjetII", "[processes]") {

  SECTION("Corsika -> QgsjetII") {
    CHECK(process::qgsjetII::ConvertToQgsjetII(PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(Proton::GetCode()) == 2);
  }

  SECTION("QgsjetII -> Corsika") {
    CHECK(PiPlus::GetCode() == process::qgsjetII::ConvertFromQgsjetII(
                                   process::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    CHECK(process::qgsjetII::ConvertToQgsjetII(PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(Proton::GetCode()) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    CHECK(process::qgsjetII::CanInteract(Proton::GetCode()));
    CHECK(process::qgsjetII::CanInteract(Code::KPlus));
    CHECK(process::qgsjetII::CanInteract(Nucleus::GetCode()));
    // CHECK(process::qgsjetII::CanInteract( Helium::GetCode()));

    CHECK_FALSE(process::qgsjetII::CanInteract(EtaC::GetCode()));
    CHECK_FALSE(process::qgsjetII::CanInteract(SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    CHECK(process::qgsjetII::GetQgsjetIIXSCode(Code::Neutron) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(Code::K0Long) ==
          process::qgsjetII::QgsjetIIXSClass::Kaons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(Code::Proton) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(Code::PiMinus) ==
          process::qgsjetII::QgsjetIIXSClass::LightMesons);
  }
}

#include <corsika/framework/geometry/Point.h>
#include <corsika/framework/geometry/RootCoordinateSystem.h>
#include <corsika/framework/geometry/Vector.h>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/framework/coreParticleProperties.h>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/qgsjetII/qgsjet-II-04.h>

using namespace corsika::units::si;
using namespace corsika::units;

TEST_CASE("QgsjetIIInterface", "[processes]") {

  // setup environment, geometry
  environment::Environment<environment::IMediumModel> env;
  auto& universe = *(env.GetUniverse());

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(std::vector<Code>{Code::Oxygen},
                                      std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  const geometry::CoordinateSystem& cs = env.GetCoordinateSystem();

  corsika::random::RNGManager::GetInstance().RegisterRandomStream("qgsjet");

  SECTION("InteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 = sqrt(E0 * E0 - Proton::GetMass() * Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                   geometry::Point, units::si::TimeType>{Code::Proton, E0, plab, pos,
                                                         0_ns});

    particle.SetNode(nodePtr);
    setup::StackView view(particle);
    auto projectile = view.GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(view);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);

    CHECK(length / (1_g / square(1_cm)) == Approx(93.47).margin(0.1));

    /***********************************
     It as turned out already two times (#291 and #307) that the detailed output of
    QGSJetII event generation depends on the gfortran version used. This is not reliable
    and cannot be tested in a unit test here. One related problem was already found (#291)
    and is realted to undefined behaviour in the evaluation of functions in logical
    expressions. It is not clear if #307 is the same issue.

     CHECK(view.GetSize() == 14);
     CHECK(sumCharge(view) == 2);
    ************************************/
    auto const secMomSum = sumMomentum(view, projectileMomentum.GetCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
          Approx(0).margin(1e-2));
  }
}
