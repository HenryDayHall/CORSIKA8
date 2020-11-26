/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/cascade/Cascade.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/energy_loss/EnergyLoss.h>
#include <corsika/process/stack_inspector/StackInspector.h>

#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/ShowerAxis.h>

#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <corsika/utl/CorsikaFenv.h>
#include <corsika/logging/Logging.h>

#include <iostream>
#include <limits>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::particles;
using namespace corsika::random;
using namespace corsika::geometry;
using namespace corsika::environment;

using namespace std;
using namespace corsika::units::si;

//
// The example main program for a particle cascade
//
int main() {

  logging::SetLevel(logging::level::info);

  C8LOG_INFO("vertical_EAS");

  std::cout << "cascade_example" << std::endl;

  const LengthType height_atmosphere = 112.8_km;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  setup::Environment env;
  auto& universe = *(env.GetUniverse());

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  auto world = setup::Environment::CreateNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

  using MyHomogeneousModel =
      environment::MediumPropertyModel<environment::UniformMagneticField<
          environment::HomogeneousMedium<setup::EnvironmentInterface>>>;

  // fraction of oxygen
  const float fox = 0.20946;
  auto const props = world->SetModelProperties<MyHomogeneousModel>(
      environment::Medium::AirDry1Atm, Vector(rootCS, 0_T, 0_T, 0_T),
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Nitrogen,
                                       particles::Code::Oxygen},
          std::vector<float>{1.f - fox, fox}));

  auto innerMedium =
      setup::Environment::CreateNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5000_m);

  innerMedium->SetModelProperties(props);
  world->AddChild(std::move(innerMedium));
  universe.AddChild(std::move(world));

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Nucleus;
  const int nuclA = 4;
  const int nuclZ = int(nuclA / 2.15 + 0.7);
  const HEPMassType mass = GetNucleusMass(nuclA, nuclZ);
  const HEPEnergyType E0 = nuclA * 1_TeV;
  double theta = 0.;
  double phi = 0.;

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      height_atmosphere); // this is the CORSIKA 7 start of atmosphere/universe

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
    auto plab = corsika::stack::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
        beamCode, E0, plab, injectionPos, 0_ns, nuclA, nuclZ});
  }

  // setup processes, decays and interactions
  setup::Tracking tracking;
  stack_inspector::StackInspector<setup::Stack> stackInspect(1, true, E0);

  random::RNGManager::GetInstance().RegisterRandomStream("sibyll");
  random::RNGManager::GetInstance().RegisterRandomStream("pythia");
  process::sibyll::Interaction sibyll;
  process::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  process::sibyll::Decay decay;
  // cascade with only HE model ==> HE cut
  process::particle_cut::ParticleCut cut(80_GeV, true, true);

  process::track_writer::TrackWriter trackWriter("tracks.dat");
  process::energy_loss::EnergyLoss eLoss{showerAxis, cut.GetECut()};

  // assemble all processes into an ordered process list
  auto sequence =
      process::sequence(stackInspect, sibyll, sibyllNuc, decay, eLoss, cut, trackWriter);

  // define air shower object, run simulation
  cascade::Cascade EAS(env, tracking, sequence, stack);

  EAS.Run();

  eLoss.PrintProfile(); // print longitudinal profile

  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  cout << "total dEdX energy (GeV): " << eLoss.GetTotal() / 1_GeV << endl
       << "relative difference (%): " << eLoss.GetTotal() / E0 * 100 << endl;
  cut.Reset();
}
