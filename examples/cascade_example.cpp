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

#include <corsika/output/OutputManager.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
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
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>

#include <iostream>
#include <limits>

using namespace corsika;
using namespace std;

//
// The example main program for a particle cascade
//
int main() {

  logging::set_level(logging::level::info);

  std::cout << "cascade_example" << std::endl;

  const LengthType height_atmosphere = 112.8_km;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  RNGManager<>::getInstance().registerRandomStream("cascade");

  // setup environment, geometry
  setup::Environment env;
  auto& universe = *(env.getUniverse());

  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  auto world =
      setup::Environment::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

  using MyHomogeneousModel = MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

  // fraction of oxygen
  float const fox = 0.20946;
  auto const props = world->setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, MagneticFieldVector(rootCS, 0_T, 0_T, 0_T),
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(std::vector<Code>{Code::Nitrogen, Code::Oxygen},
                         std::vector<float>{1.f - fox, fox}));

  auto innerMedium =
      setup::Environment::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5000_m);

  innerMedium->setModelProperties(props);
  world->addChild(std::move(innerMedium));
  universe.addChild(std::move(world));

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::Nucleus;
  const int nuclA = 4;
  const int nuclZ = int(nuclA / 2.15 + 0.7);
  const HEPMassType mass = get_nucleus_mass(nuclA, nuclZ);
  const HEPEnergyType E0 = nuclA * 1_TeV;
  double theta = 0.;
  double phi = 0.;

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      height_atmosphere); // this is the CORSIKA 7 start of atmosphere/universe

  OutputManager output("cascade_outputs");

  ShowerAxis const showerAxis{injectionPos, Vector{rootCS, 0_m, 0_m, -100_km}, env};

  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
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
    stack.addParticle(std::make_tuple(beamCode, plab, injectionPos, 0_ns, nuclA, nuclZ));
  }

  // setup processes, decays and interactions
  setup::Tracking tracking;
  StackInspector<setup::Stack> stackInspect(100, true, E0);

  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("pythia");
  corsika::sibyll::Interaction sibyll;
  corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  corsika::sibyll::Decay decay;
  // cascade with only HE model ==> HE cut
  ParticleCut cut(80_GeV, true, true);

  TrackWriter trackWriter;
  output.add("tracks", trackWriter); // register TrackWriter

  BetheBlochPDG eLoss{showerAxis};

  // assemble all processes into an ordered process list
  auto sequence =
      make_sequence(stackInspect, sibyll, sibyllNuc, decay, eLoss, cut, trackWriter);

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, output, stack);

  output.startOfShower();
  EAS.run();
  output.endOfShower();

  eLoss.printProfile(); // print longitudinal profile

  cut.showResults();
  const HEPEnergyType Efinal =
      cut.getCutEnergy() + cut.getInvEnergy() + cut.getEmEnergy();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  cout << "total dEdX energy (GeV): " << eLoss.getTotal() / 1_GeV << endl
       << "relative difference (%): " << eLoss.getTotal() / E0 * 100 << endl;
  cut.reset();

  output.endOfLibrary();
}
