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
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/Sophia.hpp>
#include <corsika/modules/PROPOSAL.hpp>

#include <corsika/modules/radio/RadioProcess.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/antennas/Antenna.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/AntennaCollection.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SimplePropagator.hpp>

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
  RNGManager<>::getInstance().registerRandomStream("sophia");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  if (seed == 0) {
    std::random_device rd;
    seed = rd();
    cout << "new random seed (auto) " << seed << endl;
  }
  RNGManager<>::getInstance().setSeed(seed);
}

template <typename TInterface>
using MyExtraEnv =
    UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;

int main(int argc, char** argv) {

  logging::set_level(logging::level::warn);

  if (argc != 3) {
    std::cerr
        << "usage: radio_em_shower <energy/GeV> <seed> - put seed=0 to use random seed"
        << std::endl;
    return 1;
  }

  int seed{static_cast<int>(std::stof(std::string(argv[2])))};
  std::cout << "Seed: " << seed << std::endl;
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  registerRandomStreams(seed);

  // setup environment, geometry
  using EnvironmentInterface =
      IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
  using EnvType = Environment<EnvironmentInterface>;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};

  create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
      env, AtmosphereId::LinsleyUSStd, center, 1.000327, Medium::AirDry1Atm,
      MagneticFieldVector{rootCS, 50_uT, 0_T, 0_T});

  std::unordered_map<Code, HEPEnergyType> energy_resolution = {
      {Code::Electron, 5_MeV},
      {Code::Positron, 5_MeV},
      {Code::Photon, 5_MeV},
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

  auto const observationHeight = 1.4_km + constants::EarthRadius::Mean;
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

  TimeType const groundHitTime{(showerCore - injectionPos).getNorm() / constants::c};

  std::string outname_ = "radio_em_shower_outputs"; // + std::to_string(rr_);
  OutputManager output(outname_);

  // Radio antennas and relevant information
  // the antenna time variables
  const TimeType duration_{1e-6_s};
  const InverseTimeType sampleRate_{1e+9_Hz};

  // the detector (aka antenna collection) for CoREAS and ZHS
  AntennaCollection<TimeDomainAntenna> detectorCoREAS;
  AntennaCollection<TimeDomainAntenna> detectorZHS;

  auto const showerCoreX_{showerCore.getCoordinates().getX()};
  auto const showerCoreY_{showerCore.getCoordinates().getY()};
  auto const injectionPosX_{injectionPos.getCoordinates().getX()};
  auto const injectionPosY_{injectionPos.getCoordinates().getY()};
  auto const injectionPosZ_{injectionPos.getCoordinates().getZ()};
  auto const triggerpoint_{Point(rootCS, injectionPosX_, injectionPosY_, injectionPosZ_)};
  std::cout << "Trigger Point is: " << triggerpoint_ << std::endl;

  // // setup CoREAS antennas - use the for loop for star shape pattern
  // for (auto radius_1 = 25_m; radius_1 <= 500_m; radius_1 += 25_m) {
  //   for (auto phi_1 = 0; phi_1 <= 315; phi_1 += 45) {
  auto radius_1 = 200_m;
  auto phi_1 = 45;
  auto phiRad_1 = phi_1 / 180. * M_PI;
  auto rr_1 = static_cast<int>(radius_1 / 1_m);
  auto const point_1{Point(rootCS, showerCoreX_ + radius_1 * cos(phiRad_1),
                           showerCoreY_ + radius_1 * sin(phiRad_1),
                           constants::EarthRadius::Mean)};
  std::cout << "Antenna point: " << point_1 << std::endl;
  auto triggertime_1{(triggerpoint_ - point_1).getNorm() / constants::c};
  std::string name_1 =
      "CoREAS_R=" + std::to_string(rr_1) + "_m--Phi=" + std::to_string(phi_1) + "degrees";
  TimeDomainAntenna antenna_1(name_1, point_1, rootCS, triggertime_1, duration_,
                              sampleRate_, triggertime_1);
  detectorCoREAS.addAntenna(antenna_1);
  //   }
  // }

  // // setup ZHS antennas - use the for loop for star shape pattern
  // for (auto radius_ = 25_m; radius_ <= 500_m; radius_ += 25_m) {
  //   for (auto phi_ = 0; phi_ <= 315; phi_ += 45) {
  auto radius_ = 200_m;
  auto phi_ = 45;
  auto phiRad_ = phi_ / 180. * M_PI;
  auto rr_ = static_cast<int>(radius_ / 1_m);
  auto const point_{Point(rootCS, showerCoreX_ + radius_ * cos(phiRad_),
                          showerCoreY_ + radius_ * sin(phiRad_),
                          constants::EarthRadius::Mean)};
  auto triggertime_{(triggerpoint_ - point_).getNorm() / constants::c};
  std::string name_ =
      "ZHS_R=" + std::to_string(rr_) + "_m--Phi=" + std::to_string(phi_) + "degrees";
  TimeDomainAntenna antenna_(name_, point_, rootCS, triggertime_, duration_, sampleRate_,
                             triggertime_);
  detectorZHS.addAntenna(antenna_);
  //   }
  // }

  // setup processes, decays and interactions

  EnergyLossWriter dEdX{showerAxis, 10_g / square(1_cm), 200};
  // register energy losses as output
  output.add("dEdX", dEdX);

  ParticleCut<SubWriter<decltype(dEdX)>> cut(5_MeV, 5_MeV, 100_GeV, 100_GeV, true, dEdX);

  corsika::sophia::InteractionModel sophia;

  corsika::sibyll::Interaction sibyll{env};
  HEPEnergyType heThresholdNN = 80_GeV;
  corsika::proposal::Interaction emCascade(
      env, sophia, sibyll.getHadronInteractionModel(), heThresholdNN);
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

  // the radio signal propagator
  auto SP = make_simple_radio_propagator(env);

  // initiate CoREAS
  RadioProcess<decltype(detectorCoREAS),
               CoREAS<decltype(detectorCoREAS), decltype(SP)>,
               decltype(SP)>
      coreas(detectorCoREAS, SP);

  // register CoREAS with the output manager
  output.add("CoREAS", coreas);

  // initiate ZHS
  RadioProcess<decltype(detectorZHS),
               ZHS<decltype(detectorZHS), decltype(SP)>,
               decltype(SP)>
      zhs(detectorZHS, SP);

  // register ZHS with the output manager
  output.add("ZHS", zhs);

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane<setup::Tracking, ParticleWriterParquet> observationLevel{
      obsPlane, DirectionVector(rootCS, {1., 0., 0.})};
  output.add("particles", observationLevel);

  // auto sequence = make_sequence(emCascade, emContinuous, longprof, cut, coreas, zhs);
  auto sequence = make_sequence(emCascade, emContinuous, longprof, coreas, zhs, tracks,
                                observationLevel, cut);
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