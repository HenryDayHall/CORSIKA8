/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/EnergyMomentumOperations.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/EnergyLossWriter.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TrackWriter.hpp>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/Random.hpp>

#include <iostream>
#include <limits>

using namespace corsika;
using namespace std;

//
// The example main program for a particle cascade
//
int main() {

  logging::set_level(logging::level::warn);

  CORSIKA_LOG_INFO("cascade_example");

  LengthType const height_atmosphere = 112.8_km;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  RNGManager<>::getInstance().registerRandomStream("cascade");

  // setup environment, geometry
  using EnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
  using EnvType = Environment<EnvironmentInterface>;
  EnvType env;
  auto& universe = *(env.getUniverse());

  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  auto world = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

  using MyHomogeneousModel =
      MediumPropertyModel<UniformMagneticField<HomogeneousMedium<EnvironmentInterface>>>;

  // fraction of oxygen
  double const fox = 0.20946;
  auto const props = world->setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, MagneticFieldVector(rootCS, 0_T, 0_T, 0_T),
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition({Code::Nitrogen, Code::Oxygen}, {1. - fox, fox}));

  auto innerMedium = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5000_m);

  innerMedium->setModelProperties(props);
  world->addChild(std::move(innerMedium));
  universe.addChild(std::move(world));

  // setup particle stack, and add primary particle
  setup::Stack<EnvType> stack;
  stack.clear();
  const int nuclA = 4;
  const int nuclZ = int(nuclA / 2.15 + 0.7);
  const Code beamCode = get_nucleus_code(nuclA, nuclZ);
  const HEPMassType mass = get_nucleus_mass(beamCode);
  const HEPEnergyType E0 = nuclA * 1_TeV;
  double theta = 0.;
  double phi = 0.;

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      height_atmosphere); // this is the CORSIKA 7 start of atmosphere/universe

  OutputManager output("cascade_outputs");

  theta *= M_PI / 180.;
  phi *= M_PI / 180.;
  DirectionVector const direction(
      rootCS, {sin(theta) * cos(phi), sin(theta) * sin(phi), -cos(theta)});

  ShowerAxis const showerAxis{injectionPos, direction * 100_km, env};
  EnergyLossWriter dEdX{showerAxis};
  output.add("energyloss", dEdX);

  {
    HEPMomentumType const P0 = calculate_momentum(E0, mass);
    auto plab = direction * P0;
    CORSIKA_LOG_INFO("input particle: {}", beamCode);
    CORSIKA_LOG_INFO("input angles: theta={} phi={}", theta, phi);
    CORSIKA_LOG_INFO("input momentum: {}", plab.getComponents() / 1_GeV);
    stack.addParticle(std::make_tuple(
        beamCode, calculate_kinetic_energy(plab.getNorm(), get_mass(beamCode)), direction,
        injectionPos, 0_ns));
  }

  // setup processes, decays and interactions
  setup::Tracking tracking;
  StackInspector<setup::Stack<EnvType>> stackInspect(100, true, E0);

  RNGManager<>::getInstance().registerRandomStream("sibyll");

  corsika::sibyll::Interaction sibyll{env};
  corsika::sibyll::Decay decay;

  // cascade with only HE model ==> HE cut
  ParticleCut<SubWriter<decltype(dEdX)>> cut(80_GeV, true, dEdX);
  BetheBlochPDG<SubWriter<decltype(dEdX)>> eLoss{dEdX};

  TrackWriter trackWriter;
  output.add("tracks", trackWriter); // register TrackWriter

  // assemble all processes into an ordered process list
  auto sequence = make_sequence(stackInspect, sibyll, decay, eLoss, trackWriter, cut);

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, output, stack);

  output.startOfLibrary();
  EAS.run();
  output.endOfLibrary();

  const HEPEnergyType Efinal = dEdX.getEnergyLost();
  CORSIKA_LOG_INFO(
      "\n"
      "total cut energy (GeV) : {}\n"
      "relative difference (%): {}\n"
      "total dEdX energy (GeV): {}\n"
      "relative difference (%): {}\n",
      Efinal / 1_GeV, (Efinal / E0 - 1) * 100, dEdX.getEnergyLost() / 1_GeV,
      dEdX.getEnergyLost() / E0 * 100);
}
