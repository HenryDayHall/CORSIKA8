/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/EnergyMomentumOperations.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/utility/SaveBoostHistogram.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/EnergyLossWriter.hpp>
#include <corsika/modules/writers/LongitudinalWriter.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/Sibyll.hpp>
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
 number generator. It has to occur exactly ONCE per linked
 executable. If you include the header below multiple times and
 link this together, it will fail.
*/
#include <corsika/modules/Random.hpp>

using namespace corsika;
using namespace std;

void registerRandomStreams(int seed) {
  RNGManager<>::getInstance().registerRandomStream("cascade");
  RNGManager<>::getInstance().registerRandomStream("proposal");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  if (seed == 0) {
    std::random_device rd;
    seed = rd();
    cout << "new random seed (auto) " << seed << endl;
  }
  RNGManager<>::getInstance().setSeed(seed);
}

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

int main(int argc, char** argv) {

  logging::set_level(logging::level::info);

  if (argc != 3) {
    std::cerr << "usage: em_shower <energy/GeV> [seed]" << std::endl
              << "seed = 0 for randomized seed" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);
  int seed = 0;

  if (argc > 2) { seed = std::stoi(std::string(argv[2])); }
  // initialize random number sequence(s)
  registerRandomStreams(seed);

  // setup environment, geometry
  using EnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
  using EnvType = Environment<EnvironmentInterface>;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};

  // build a Linsley US Standard atmosphere into `env`
  create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
      env, AtmosphereId::LinsleyUSStd, center, Medium::AirDry1Atm,
      MagneticFieldVector{rootCS, 20.4_uT, 0_T, 43.23_uT});

  std::unordered_map<Code, HEPEnergyType> energy_resolution = {
      {Code::Electron, 2_MeV},
      {Code::Positron, 2_MeV},
      {Code::Photon, 2_MeV},
  };
  for (auto [pcode, energy] : energy_resolution)
    set_energy_production_threshold(pcode, energy);

  // setup particle stack, and add primary particle
  setup::Stack<EnvType> stack;
  stack.clear();

  const Code beamCode = Code::Electron;
  auto const mass = get_mass(beamCode);
  const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[1]));
  double theta = 0.;
  auto const thetaRad = theta / 180. * M_PI;

  HEPMomentumType P0 = calculate_momentum(E0, mass);
  auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
  };

  auto const [px, py, pz] = momentumComponents(thetaRad, P0);
  auto plab = MomentumVector(rootCS, {px, py, pz});
  cout << "input particle: " << beamCode << endl;
  cout << "input angles: theta=" << theta << endl;
  cout << "input momentum: " << plab.getComponents() / 1_GeV
       << ", norm = " << plab.getNorm() << endl;

  auto const observationHeight = 0.0_km + constants::EarthRadius::Mean;
  auto const injectionHeight = 112.75_km + constants::EarthRadius::Mean;
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

  stack.addParticle(std::make_tuple(
      beamCode, calculate_kinetic_energy(plab.getNorm(), get_mass(beamCode)),
      plab.normalized(), injectionPos, 0_ns));

  CORSIKA_LOG_INFO("shower axis length: {} ",
                   (showerCore - injectionPos).getNorm() * 1.02);

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.02, env,
                              false, 1000};

  OutputManager output("em_shower_outputs");

  EnergyLossWriter dEdX{showerAxis, 10_g / square(1_cm), 200};
  // register energy losses as output
  output.add("dEdX", dEdX);

  // setup processes, decays and interactions

  ParticleCut<SubWriter<decltype(dEdX)>> cut(2_MeV, 2_MeV, 100_GeV, 100_GeV, true, dEdX);
  corsika::sibyll::Interaction sibyll{env};
  HEPEnergyType heThresholdNN = 60_GeV;
  corsika::proposal::Interaction emCascade(env, sibyll.getHadronInteractionModel(),
                                           heThresholdNN);
  corsika::proposal::ContinuousProcess<SubWriter<decltype(dEdX)>> emContinuous(env, dEdX);
  //  BetheBlochPDG<SubWriter<decltype(dEdX)>> emContinuous{dEdX};

  //  NOT possible right now, due to interface differenc in PROPOSAL
  //  InteractionCounter emCascadeCounted(emCascade);

  TrackWriter tracks;
  output.add("tracks", tracks);

  // long. profile
  LongitudinalWriter profile{showerAxis, 10_g / square(1_cm)};
  output.add("profile", profile);
  LongitudinalProfile<SubWriter<decltype(profile)>> longprof{profile};

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane<setup::Tracking, ParticleWriterParquet> observationLevel{
      obsPlane, DirectionVector(rootCS, {1., 0., 0.})};
  output.add("particles", observationLevel);

  auto sequence = make_sequence(emCascade, emContinuous, longprof, cut, observationLevel);
  // define air shower object, run simulation
  setup::Tracking tracking;

  output.startOfLibrary();
  Cascade EAS(env, tracking, sequence, output, stack);

  // to fix the point of first interaction, uncomment the following two lines:
  //  EAS.forceInteraction();

  EAS.run();

  HEPEnergyType const Efinal = dEdX.getEnergyLost() + observationLevel.getEnergyGround();

  CORSIKA_LOG_INFO(
      "total energy budget (GeV): {}, "
      "relative difference (%): {}",
      Efinal / 1_GeV, (Efinal / E0 - 1) * 100);

  output.endOfLibrary();
}
