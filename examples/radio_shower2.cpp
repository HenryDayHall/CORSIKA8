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

  std::cout << "radio_shower2" << std::endl;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  RNGManager::getInstance().registerRandomStream("cascade");

  // This environment needs a hardcoded refractive index in the propagator at the moment
  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  auto& universe = *(env.getUniverse());
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  // the antenna location
  const auto point1{Point(rootCS, 50_m, 50_m, 0_m)};
  const auto point2{Point(rootCS, 50_m, -50_m, 0_m)};
  const auto point3{Point(rootCS, -50_m, 50_m, 0_m)};
  const auto point4{Point(rootCS, -50_m, -50_m, 0_m)};

  // the antennas
  TimeDomainAntenna ant1("antenna1", point1, 0_s, 1_s, 1/1e+6_s);
  TimeDomainAntenna ant2("antenna2", point2, 0_s, 1_s, 1/1e+6_s);
  TimeDomainAntenna ant3("antenna3", point3, 0_s, 1_s, 1/1e+6_s);
  TimeDomainAntenna ant4("antenna4", point4, 0_s, 1_s, 1/1e+6_s);

  // the detector
  AntennaCollection<TimeDomainAntenna> detector;
  detector.addAntenna(ant1);
  detector.addAntenna(ant2);
  detector.addAntenna(ant3);
  detector.addAntenna(ant4);

  auto world = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

  using MyHomogeneousModel = MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

  world->setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, MagneticFieldVector(rootCS, 0_T, 0_T, 1_T),
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(std::vector<Code>{Code::Hydrogen},
                         std::vector<float>{(float)1.}));

  universe.addChild(std::move(world));

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::Electron;
  const HEPMassType mass = Electron::mass;
  const HEPEnergyType E0 = 1000_GeV;
  double theta = 0.;
  double phi = 0.;

  Point injectionPos(rootCS, 0_m, 0_m, 0_m);
  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt(Elab * Elab - m * m);
    };
    HEPMomentumType P0 = elab2plab(E0, mass);
    auto momentumComponents = [](double theta, double phi, HEPMomentumType ptot) {
      return std::make_tuple(ptot * sin(theta) * cos(phi), ptot * sin(theta) * sin(phi),
                             -ptot * cos(theta));
    };
    auto const [px, py, pz] =
    momentumComponents(theta / 180. * M_PI, phi / 180. * M_PI, P0);
    auto plab = MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.getComponents() / 1_GeV << endl;
    stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns));
  }

  // setup processes, decays and interactions
  setup::Tracking tracking;
  StackInspector<setup::Stack> stackInspect(1, true, E0);

  // put radio process here
  RadioProcess<decltype(detector), CoREAS<decltype(detector),
      decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
      coreas(detector, env);


  TrackWriter trackWriter("tracks.dat");
  ShowerAxis const showerAxis{injectionPos, Vector{rootCS, 0_m, 0_m, -100_km}, env};

  // assemble all processes into an ordered process list
  auto sequence = make_sequence(coreas, trackWriter, stackInspect);

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, stack);
  EAS.run();

  // get radio output
  coreas.writeOutput();
}
