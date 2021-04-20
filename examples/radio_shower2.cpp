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

  world->setModelProperties<MyHomogeneousModel>(1,
                                                Medium::AirDry1Atm, MagneticFieldVector(rootCS, 0_T, 0_T, 0.3809_T),
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
  const HEPMassType mass = Electron::mass;
  const HEPEnergyType E0 = 11.4_MeV;
  double theta = 0.;
  double phi = 0.;

  Point injectionPos(rootCS, 0_m, 100_m, 0_m);
  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt(Elab * Elab - m * m);
    };
    HEPMomentumType P0 = elab2plab(E0, mass);
    auto momentumComponents = [](double theta, double phi, HEPMomentumType ptot) {
      return std::make_tuple(ptot * cos(theta), ptot * sin(theta),
                             ptot * sin(theta));
    };
    auto const [px, py, pz] =
    momentumComponents(theta / 180. * M_PI, phi / 180. * M_PI, P0);
    auto plab = MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input momentum: " << plab.getComponents() / 1_GeV << endl;
    stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns));
  }

  // setup relevant processes
  setup::Tracking tracking;
//  StackInspector<setup::Stack> stackInspect(1, true, E0);

  // put radio process here
  RadioProcess<decltype(detector), CoREAS<decltype(detector),
      decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
      coreas(detector, env);

  TimeCut cut(1e-9_s);

  TrackWriter trackWriter("tracks.dat");

  // assemble all processes into an ordered process list
  auto sequence = make_sequence(coreas, cut, trackWriter);

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, stack);
  EAS.run();

  // get radio output
  coreas.writeOutput();
}
