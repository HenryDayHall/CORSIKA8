/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/utility/SaveBoostHistogram.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/PROPOSAL.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpppp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>

using namespace corsika;
using namespace std;

void registerRandomStreams() {
  RNGManager::getInstance().registerRandomStream("cascade");
  RNGManager::getInstance().registerRandomStream("proposal");
  RNGManager::getInstance().seedAll();
}

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

int main(int argc, char** argv) {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  if (argc != 2) {
    std::cerr << "usage: em_shower <energy/GeV>" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  registerRandomStreams();

  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  auto builder = make_layered_spherical_atmosphere_builder<
      setup::EnvironmentInterface, MyExtraEnv>::create(center,
                                                       constants::EarthRadius::Mean,
                                                       Medium::AirDry1Atm,
                                                       Vector{rootCS, 0_T, 50_uT, 0_T});
  builder.setNuclearComposition(
      {{Code::Nitrogen, Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);
  builder.assemble(env);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::Electron;
  auto const mass = get_mass(beamCode);
  const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[1]));
  double theta = 0.;
  auto const thetaRad = theta / 180. * M_PI;

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };
  HEPMomentumType P0 = elab2plab(E0, mass);
  auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
  };

  auto const [px, py, pz] = momentumComponents(thetaRad, P0);
  auto plab = MomentumVector(rootCS, {px, py, pz});
  cout << "input particle: " << beamCode << endl;
  cout << "input angles: theta=" << theta << endl;
  cout << "input momentum: " << plab.getComponents() / 1_GeV
       << ", norm = " << plab.getNorm() << endl;

  auto const observationHeight = 1.4_km + builder.getEarthRadius();
  auto const injectionHeight = 112.75_km + builder.getEarthRadius();
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

  stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns));

  std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.02
            << std::endl;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.02, env};

  // setup processes, decays and interactions

  // PROPOSAL processs proposal{...};
  ParticleCut cut(10_GeV, 10_GeV, 100_PeV, 100_PeV, true);
  corsika::proposal::Interaction proposal(env, cut.getElectronECut());
  corsika::proposal::ContinuousProcess em_continuous(env, cut.getElectronECut());
  InteractionCounter proposalCounted(proposal);

  TrackWriter trackWriter("tracks.dat");

  // long. profile; columns for gamma, e+, e- still need to be added
  LongitudinalProfile longprof{showerAxis};

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane observationLevel(obsPlane, DirectionVector(rootCS, {1., 0., 0.}),
                                    "particles.dat");

  auto sequence = make_sequence(proposalCounted, em_continuous, longprof, cut,
                                observationLevel, trackWriter);
  // define air shower object, run simulation
  setup::Tracking tracking;
  Cascade EAS(env, tracking, sequence, stack);

  // to fix the point of first interaction, uncomment the following two lines:
  //  EAS.setNodes();
  //  EAS.forceInteraction();

  EAS.run();

  cut.showResults();
  em_continuous.showResults();
  observationLevel.showResults();
  const HEPEnergyType Efinal = cut.getCutEnergy() + cut.getInvEnergy() +
                               cut.getEmEnergy() + em_continuous.getEnergyLost() +
                               observationLevel.getEnergyGround();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  observationLevel.reset();
  cut.reset();
  em_continuous.reset();

  auto const hists = proposalCounted.getHistogram();
  save_hist(hists.labHist(), "inthist_lab_emShower.npz", true);
  save_hist(hists.CMSHist(), "inthist_cms_emShower.npz", true);
  longprof.save("longprof_emShower.txt");
}
