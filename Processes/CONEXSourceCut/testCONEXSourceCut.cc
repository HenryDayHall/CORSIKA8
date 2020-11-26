/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/setup/SetupEnvironment.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
#include <corsika/environment/MediumPropertyModel.h>
#include <corsika/environment/UniformMagneticField.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/process/conex_source_cut/CONEXSourceCut.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/random/RNGManager.h>

#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/CorsikaFenv.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::environment;
using namespace corsika::geometry;
using namespace corsika::units::si;

const std::string refDataDir = std::string(REFDATADIR); // from cmake

template <typename T>
using MExtraEnvirnoment =
    environment::MediumPropertyModel<environment::UniformMagneticField<T>>;

TEST_CASE("CONEXSourceCut") {
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");
  random::RNGManager::GetInstance().RegisterRandomStream("sibyll");

  feenableexcept(FE_INVALID);

  // setup environment, geometry
  setup::Environment env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};

  auto builder = environment::make_layered_spherical_atmosphere_builder<
      setup::EnvironmentInterface,
      MExtraEnvirnoment>::create(center, conex::earthRadius,
                                 environment::Medium::AirDry1Atm,
                                 geometry::Vector{rootCS, 0_T, 50_mT, 0_T});

  builder.setNuclearComposition(
      {{particles::Code::Nitrogen, particles::Code::Oxygen},
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

  auto const observationHeight = 1.4_km + conex::earthRadius;
  auto const injectionHeight = 112.75_km + conex::earthRadius;
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-units::static_pow<2>(sin(thetaRad) * observationHeight) +
                      units::static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore +
      Vector<dimensionless_d>{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  environment::ShowerAxis const showerAxis{injectionPos,
                                           (showerCore - injectionPos) * 1.02, env};

  // need to initialize Sibyll, done in constructor:
  process::sibyll::Interaction sibyll;
  [[maybe_unused]] process::sibyll::NuclearInteraction sibyllNuc(sibyll, env);

  corsika::process::conex_source_cut::CONEXSourceCut conex(
      center, showerAxis, t, injectionHeight, E0,
      particles::GetPDG(particles::Code::Proton));

  HEPEnergyType const Eem{1_PeV};
  auto const momentum = showerAxis.GetDirection() * Eem;

  auto const emPosition = showerCore + showerAxis.GetDirection() * (-20_km);

  std::cout << "position injection: "
            << injectionPos.GetCoordinates(conex.GetObserverCS()) << " "
            << injectionPos.GetCoordinates(rootCS) << std::endl;
  std::cout << "position core: " << showerCore.GetCoordinates(conex.GetObserverCS())
            << " " << showerCore.GetCoordinates(rootCS) << std::endl;
  std::cout << "position EM: " << emPosition.GetCoordinates(conex.GetObserverCS()) << " "
            << emPosition.GetCoordinates(rootCS) << std::endl;

  conex.addParticle(particles::Code::Proton, Eem, 0_eV, emPosition, momentum.normalized(),
                    0_s);
  // supperimpose a photon
  auto const momentumPhoton = showerAxis.GetDirection() * 1_TeV;
  conex.addParticle(particles::Code::Gamma, 1_TeV, 0_eV, emPosition,
                    momentumPhoton.normalized(), 0_s);
  conex.SolveCE();
}

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>

TEST_CASE("ConexOutput", "[output validation]") {

  auto file = GENERATE(as<std::string>{}, "conex_fit", "conex_output");

  SECTION(std::string("check saved data, ") + file + ".txt") {

    // compare to binary reference data
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
