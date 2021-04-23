/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/geometry/Sphere.hpp>

#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/modules/radio/RadioProcess.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/antennas/Antenna.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/RadioDetector.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TimeCut.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/HadronicElasticModel.hpp>
#include <corsika/modules/Pythia8.hpp>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpppp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>

#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace std;

//
// A simple shower to get the electric field trace of an electron
//
int main() {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  std::cout << "Synchrotron radiation" << std::endl;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  RNGManager::getInstance().registerRandomStream("cascade");

  // This environment may need a hardcoded refractive index in the propagator at the moment although it shouldn't
  // set up the environment
  using EnvType = setup::Environment;
  EnvType env;
  auto& universe = *(env.getUniverse());
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  auto world = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

  using MyHomogeneousModel = UniformRefractiveIndex<MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>>;

  auto const Bmag {0.3809_T};
  MagneticFieldVector B{rootCS, 0_T, 0_T, Bmag};

  world->setModelProperties<MyHomogeneousModel>(1,
                                                Medium::AirDry1Atm, B,
                                                1_kg / (1_m * 1_m * 1_m),
                                                NuclearComposition(std::vector<Code>{Code::Nitrogen},
                                                                   std::vector<float>{(float)1.}));

  universe.addChild(std::move(world));

//  // The following environment is the same as the one above qualitatively but it doesn't compile with Cascade.
//  //  I leave it here for now, as I am curious to understand why at some point.
//  using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
//  using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
//      <IModelInterface>>>>;
//  using EnvType = Environment<AtmModel>;
//  EnvType env;
//  auto& universe = *(env.getUniverse());
//  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
//
//  auto world = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);
//
//  world->setModelProperties<AtmModel>(1,
//                                                Medium::AirDry1Atm, MagneticFieldVector(rootCS, 0_T, 0_T, 0.3809_T),
//                                                1_kg / (1_m * 1_m * 1_m),
//                                                NuclearComposition(std::vector<Code>{Code::Nitrogen},
//                                                                   std::vector<float>{(float)1.}));
//
//  universe.addChild(std::move(world));

  // the antenna locations
  const auto point1{Point(rootCS, 100_m, 100_m, 0_m)};
  const auto point2{Point(rootCS, 100_m, -100_m, 0_m)};
  const auto point3{Point(rootCS, -100_m, -100_m, 0_m)};
  const auto point4{Point(rootCS, -100_m, 100_m, 0_m)};

  // the antenna time variables
  const TimeType t1{0_s};
  const TimeType t2{1e-6_s};
  const InverseTimeType t3{1e+9_Hz};

  // the antennas
  TimeDomainAntenna ant1("antenna 1", point1, t1, t2, t3);
  TimeDomainAntenna ant2("antenna 2", point2, t1, t2, t3);
  TimeDomainAntenna ant3("antenna 3", point3, t1, t2, t3);
  TimeDomainAntenna ant4("antenna 4", point4, t1, t2, t3);

  // the detector
  AntennaCollection<TimeDomainAntenna> detector;
  detector.addAntenna(ant1);
  detector.addAntenna(ant2);
  detector.addAntenna(ant3);
  detector.addAntenna(ant4);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::Electron;
  auto const gyroradius = 100_m;
  auto const pLabMag = convert_SI_to_HEP(get_charge(beamCode) * Bmag * gyroradius);
  auto const omega_inv = convert_HEP_to_SI<MassType::dimension_type>(get_mass(beamCode)) / ((-1)*get_charge(beamCode) * Bmag);
  MomentumVector const plab{rootCS, pLabMag, 0_MeV, 0_MeV};
  auto const Elab = sqrt(plab.getSquaredNorm() + static_pow<2>(get_mass(beamCode)));
  TimeType const period = 2 * M_PI * omega_inv;

  std::cout << "|p| = " << plab.getNorm() << "; E = " << Elab << std::endl;
  std::cout << "period: " << period << std::endl;

  Point injectionPos(rootCS, 0_m, 0_m, 0_m);
  stack.addParticle(std::make_tuple(beamCode, Elab, plab, injectionPos, 0_ns));

  // setup relevant processes
  setup::Tracking tracking;

  // put radio process here
  RadioProcess<decltype(detector), CoREAS<decltype(detector),
      decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
      coreas(detector, env);

  TimeCut cut(period);

  TrackWriter trackWriter("tracks.dat");

  // assemble all processes into an ordered process list
  auto sequence = make_sequence(coreas, cut, trackWriter);

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, stack);
  EAS.run();

  // get radio output
  coreas.writeOutput();
}