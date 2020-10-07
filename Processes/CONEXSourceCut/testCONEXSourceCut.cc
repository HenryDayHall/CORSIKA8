/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>
#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
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

TEST_CASE("CONEXSourceCut") {
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");
  random::RNGManager::GetInstance().RegisterRandomStream("sibyll");

  feenableexcept(FE_INVALID);

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  environment::LayeredSphericalAtmosphereBuilder builder{center, conex::earthRadius};
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

  conex.addParticle(0, Eem, 0_eV, emPosition, momentum.normalized(), 0_s);

  conex.SolveCE();
}
