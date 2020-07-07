/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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
  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  environment::LayeredSphericalAtmosphereBuilder builder{center};
  builder.setNuclearComposition(
      {{particles::Code::Nitrogen, particles::Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);

  builder.assemble(env);

  const HEPEnergyType mass = particles::GetMass(particles::Code::Proton);
  const HEPEnergyType E0 = 1_PeV;
  double thetaDeg = 0.;
  auto const thetaRad = thetaDeg / 180. * M_PI;

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };

  HEPMomentumType P0 = elab2plab(E0, mass);

  auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
  };
  auto const [px, py, pz] = momentumComponents(thetaRad, P0);
  auto plab = corsika::stack::MomentumVector(rootCS, {px, py, pz});

  auto const observationHeight = 1.4_km + builder.earthRadius;
  auto const injectionHeight = 112.75_km + builder.earthRadius;
  auto const t =
      -observationHeight * cos(thetaRad) +
      sqrt(-units::si::detail::static_pow<2>(sin(thetaRad) * observationHeight) +
           units::si::detail::static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore +
      Vector<dimensionless_d>{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  environment::ShowerAxis const showerAxis{injectionPos,
                                           (showerCore - injectionPos) * 1.02, env};

  corsika::process::conex_source_cut::CONEXSourceCut(
      center, showerAxis, (showerCore - injectionPos).norm(), E0,
      particles::GetPDG(particles::Code::Proton));
}
