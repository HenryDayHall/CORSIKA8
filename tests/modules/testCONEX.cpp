/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/setup/SetupEnvironment.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/modules/CONEX.hpp>
#include <corsika/modules/Sibyll.hpp>

#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

const std::string refDataDir = std::string(REFDATADIR); // from cmake

template <typename T>
using MExtraEnvirnoment = MediumPropertyModel<UniformMagneticField<T>>;

TEST_CASE("CONEXSourceCut") {
  RNGManager::getInstance().registerRandomStream("cascade");
  RNGManager::getInstance().registerRandomStream("sibyll");

  feenableexcept(FE_INVALID);

  // setup environment, geometry
  setup::Environment env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};

  auto builder = make_layered_spherical_atmosphere_builder<
      setup::EnvironmentInterface, MExtraEnvirnoment>::create(center,
                                                              corsika::conex::earthRadius,
                                                              Medium::AirDry1Atm,
                                                              Vector{rootCS, 0_T, 50_mT,
                                                                     0_T});

  builder.setNuclearComposition(
      {{Code::Nitrogen, Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);

  builder.assemble(env);

  const HEPEnergyType E0 = 1_PeV;
  double thetaDeg = 60.;
  auto const thetaRad = thetaDeg / 180. * M_PI;

  auto const observationHeight = 1.4_km + corsika::conex::earthRadius;
  auto const injectionHeight = 112.75_km + corsika::conex::earthRadius;
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore +
      Vector<dimensionless_d>{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.02, env};

  // need to initialize Sibyll, done in constructor:
  corsika::sibyll::Interaction sibyll;
  [[maybe_unused]] corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);

  CONEXhybrid conex(center, showerAxis, t, injectionHeight, E0, get_PDG(Code::Proton));

  HEPEnergyType const Eem{1_PeV};
  auto const momentum = showerAxis.getDirection() * Eem;

  auto const emPosition = showerCore + showerAxis.getDirection() * (-20_km);

  std::cout << "position injection: "
            << injectionPos.getCoordinates(conex.getObserverCS()) << " "
            << injectionPos.getCoordinates(rootCS) << std::endl;
  std::cout << "position core: " << showerCore.getCoordinates(conex.getObserverCS())
            << " " << showerCore.getCoordinates(rootCS) << std::endl;
  std::cout << "position EM: " << emPosition.getCoordinates(conex.getObserverCS()) << " "
            << emPosition.getCoordinates(rootCS) << std::endl;

  conex.addParticle(Code::Proton, Eem, 0_eV, emPosition, momentum.normalized(), 0_s);
  // supperimpose a photon
  auto const momentumPhoton = showerAxis.getDirection() * 1_TeV;
  conex.addParticle(Code::Gamma, 1_TeV, 0_eV, emPosition, momentumPhoton.normalized(),
                    0_s);
  conex.solveCE();
}

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>

TEST_CASE("ConexOutput", "[output validation]") {

  auto file = GENERATE(as<std::string>{}, "conex_fit", "conex_output");

  SECTION(std::string("check saved data, ") + file + ".txt") {

    // compare to reference data
    std::ifstream file1(file + ".txt");
    std::ifstream file1ref(refDataDir + "/" + file + "_REF.txt");

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin1ref(file1ref);

    std::istreambuf_iterator<char> end;

    while (begin1 != end && begin1ref != end) {
      CHECK(*begin1 == *begin1ref);
      ++begin1;
      ++begin1ref;
    }
    CHECK(begin1 == end);
    CHECK(begin1ref == end);
    file1.close();
    file1ref.close();
  }
}
