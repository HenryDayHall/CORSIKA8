/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/sequence/ProcessSequence.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/modules/energy_loss/EnergyLoss.hpp>
#include <corsika/modules/particle_cut/ParticleCut.hpp>
#include <corsika/modules/sibyll/Decay.hpp>
#include <corsika/modules/sibyll/Interaction.hpp>
#include <corsika/modules/sibyll/NuclearInteraction.hpp>
#include <corsika/modules/stack_inspector/StackInspector.hpp>
#include <corsika/modules/track_writer/TrackWriter.hpp>
#include <corsika/modules/tracking_line/TrackingLine.hpp>
#include <corsika/modules/pythia8/Interaction.hpp>
#include <corsika/modules/pythia8/Decay.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace corsika::setup;
using namespace corsika::units::si;
using namespace std;

//
// The example main program for a particle cascade
//
int main() {
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  corsika::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  auto theMedium =
      EnvType::CreateNode<Sphere>(Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
                                  1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = HomogeneousMedium<IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(std::vector<corsika::Code>{corsika::Code::Hydrogen},
                         std::vector<float>{(float)1.}));

  universe.AddChild(std::move(theMedium));

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Proton;
  const HEPMassType mass = corsika::Proton::GetMass();
  const HEPEnergyType E0 = 100_GeV;
  double theta = 0.;
  double phi = 0.;

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
    auto plab = corsika::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    Point pos(rootCS, 0_m, 0_m, 0_m);
    stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType,
                   corsika::MomentumVector, corsika::Point, units::si::TimeType>{
            beamCode, E0, plab, pos, 0_ns});
  }

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;
  stack_inspector::StackInspector<setup::Stack> stackInspect(1, true, E0);

  const std::vector<corsika::Code> trackedHadrons = {
      corsika::Code::PiPlus, corsika::Code::PiMinus, corsika::Code::KPlus,
      corsika::Code::KMinus, corsika::Code::K0Long,  corsika::Code::K0Short};

  corsika::RNGManager::GetInstance().RegisterRandomStream("s_rndm");
  corsika::RNGManager::GetInstance().RegisterRandomStream("pythia");
  //  corsika::sibyll::Interaction sibyll(env);
  corsika::pythia8::Interaction pythia;
  //  corsika::sibyll::NuclearInteraction sibyllNuc(env, sibyll);
  //  corsika::sibyll::Decay decay(trackedHadrons);
  corsika::pythia8::Decay decay(trackedHadrons);
  corsika::particle_cut::ParticleCut cut(20_GeV);

  // corsika::RNGManager::GetInstance().RegisterRandomStream("HadronicElasticModel");
  // corsika::HadronicElasticModel::HadronicElasticInteraction
  // hadronicElastic(env);

  corsika::track_writer::TrackWriter trackWriter("tracks.dat");

  // assemble all processes into an ordered process list
  // auto sequence = sibyll << decay << hadronicElastic << cut << trackWriter;
  auto sequence = pythia << decay << cut << trackWriter << stackInspect;

  // cout << "decltype(sequence)=" << type_id_with_cvr<decltype(sequence)>().pretty_name()
  // << "\n";

  // define air shower object, run simulation
  corsika::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  cout << "Result: E0=" << E0 / 1_GeV << endl;
  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1.) * 100 << endl;
}
