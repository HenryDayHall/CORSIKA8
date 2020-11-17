/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/TrackingLine.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <iostream>
#include <limits>

using namespace corsika;
using namespace corsika::setup;
using namespace std;

//
// The example main program for a particle cascade
//
int main() {

  const LengthType height_atmosphere = 112.8_km;

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  corsika::RNGManager::getInstance().registerRandomStream("cascade");

  // setup environment, geometry
  using EnvType = corsika::Environment<setup::IEnvironmentModel>;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  auto outerMedium = EnvType::CreateNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  // fraction of oxygen
  const float fox = 0.20946;
  auto const props =
      outerMedium
          ->SetModelProperties<corsika::HomogeneousMedium<setup::IEnvironmentModel>>(
              1_kg / (1_m * 1_m * 1_m),
              corsika::NuclearComposition(
                  std::vector<corsika::Code>{corsika::Code::Nitrogen,
                                             corsika::Code::Oxygen},
                  std::vector<float>{1.f - fox, fox}));

  auto innerMedium = EnvType::CreateNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5000_m);

  innerMedium->SetModelProperties(props);

  outerMedium->AddChild(std::move(innerMedium));

  universe.AddChild(std::move(outerMedium));

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
    auto plab = corsika::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    stack.AddParticle(
        std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, corsika::Point,
                   TimeType, unsigned short, unsigned short>{
            beamCode, E0, plab, injectionPos, 0_ns, nuclA, nuclZ});
  }

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;
  stack_inspector::StackInspector<setup::Stack> stackInspect(1, true, E0);

  corsika::RNGManager::getInstance().registerRandomStream("sibyll");
  corsika::RNGManager::getInstance().registerRandomStream("pythia");
  corsika::sibyll::Interaction sibyll;
  corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  corsika::sibyll::Decay decay;
  // cascade with only HE model ==> HE cut
  corsika::particle_cut::ParticleCut cut(80_GeV);

  corsika::track_writer::TrackWriter trackWriter("tracks.dat");
  corsika::energy_loss::BetheBlochPDG eLoss(
      injectionPos, corsika::Vector<dimensionless_d>(rootCS, {0, 0, -1}));

  // assemble all processes into an ordered process list
  auto sequence = stackInspect << sibyll << sibyllNuc << decay << eLoss << cut
                               << trackWriter;

  // define air shower object, run simulation
  corsika::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  eLoss.PrintProfile(); // print longitudinal profile

  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  cout << "total dEdX energy (GeV): " << eLoss.GetTotal() / 1_GeV << endl
       << "relative difference (%): " << eLoss.GetTotal() / E0 * 100 << endl;
}
