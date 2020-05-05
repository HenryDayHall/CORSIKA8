/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/cascade/Cascade.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/StackProcess.h>
#include <corsika/process/energy_loss/EnergyLoss.h>
#include <corsika/process/interaction_counter/InteractionCounter.h>
#include <corsika/process/observation_plane/ObservationPlane.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/switch_process/SwitchProcess.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/pythia/Decay.h>

#include <corsika/process/urqmd/UrQMD.h>

#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <corsika/utl/CorsikaFenv.h>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::particles;
using namespace corsika::random;
using namespace corsika::setup;
using namespace corsika::geometry;
using namespace corsika::environment;

using namespace std;
using namespace corsika::units::si;

void registerRandomStreams() {
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");
  random::RNGManager::GetInstance().RegisterRandomStream("qgran");
  random::RNGManager::GetInstance().RegisterRandomStream("s_rndm");
  random::RNGManager::GetInstance().RegisterRandomStream("pythia");
  random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");

  random::RNGManager::GetInstance().SeedAll();
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "must provide A, Z, energy" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  registerRandomStreams();

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  environment::LayeredSphericalAtmosphereBuilder builder(Point{rootCS, 0_m, 0_m, 0_m});
  builder.setNuclearComposition(
      {{particles::Code::Nitrogen, particles::Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);

  builder.assemble(env);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Nucleus;
  unsigned short const A = std::stoi(std::string(argv[1]));
  unsigned short Z = std::stoi(std::string(argv[2]));
  auto const mass = particles::GetNucleusMass(A, Z);
  const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[3]));
  double theta = 0.;
  double phi = 0.;

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      112.7_km +
          builder.earthRadius); // this is the CORSIKA 7 start of atmosphere/universe

  //  {
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

  if (A != 1) {
    stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
        beamCode, E0, plab, injectionPos, 0_ns, A, Z});

  } else {
    stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, E0, plab, injectionPos, 0_ns});
  }

  Line const line(injectionPos, plab.normalized() * 1_m * 1_Hz);
  auto const velocity = line.GetV0().norm();

  auto const observationHeight = 1.4_km + builder.earthRadius;

  setup::Trajectory const showerAxis(line, (112.7_km - observationHeight) / velocity);

  // setup processes, decays and interactions

  process::sibyll::Interaction sibyll;
  process::interaction_counter::InteractionCounter sibyllCounted(sibyll);

  process::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  process::interaction_counter::InteractionCounter sibyllNucCounted(sibyllNuc);

  process::pythia::Decay decayPythia;

  // use sibyll decay routine for decays of particles unknown to pythia
  process::sibyll::Decay decaySibyll({
      Code::N1440Plus,
      Code::N1440MinusBar,
      Code::N1440_0,
      Code::N1440_0Bar,
      Code::N1710Plus,
      Code::N1710MinusBar,
      Code::N1710_0,
      Code::N1710_0Bar,

      Code::Pi1300Plus,
      Code::Pi1300Minus,
      Code::Pi1300_0,

      Code::KStar0_1430_0,
      Code::KStar0_1430_0Bar,
      Code::KStar0_1430_Plus,
      Code::KStar0_1430_MinusBar,
  });
  decaySibyll.PrintDecayConfig();

  process::particle_cut::ParticleCut cut(100_GeV);

  // process::track_writer::TrackWriter trackWriter("tracks.dat");
  process::energy_loss::EnergyLoss eLoss(showerAxis);

  Plane const obsPlane(Point(rootCS, 0_m, 0_m, observationHeight),
                       Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
  process::observation_plane::ObservationPlane observationLevel(obsPlane, "/dev/null");

  // assemble all processes into an ordered process list

  process::UrQMD::UrQMD urqmd;

  auto sibyllSequence = sibyllNucCounted << sibyllCounted;
  process::switch_process::SwitchProcess switchProcess(urqmd, sibyllSequence, 55_GeV);
  auto decaySequence = decayPythia << decaySibyll;
  auto sequence = switchProcess << decaySequence << eLoss << cut << observationLevel;
  // << trackWriter;

  // define air shower object, run simulation
  tracking_line::TrackingLine tracking;
  cascade::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  //  EAS.SetNodes();
  //  EAS.forceInteraction();
  EAS.Run();

  eLoss.PrintProfile(); // print longitudinal profile

  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  cout << "total dEdX energy (GeV): " << eLoss.GetTotal() / 1_GeV << endl
       << "relative difference (%): " << eLoss.GetTotal() / E0 * 100 << endl;

  auto const hists = sibyllCounted.GetHistogram() + sibyllNucCounted.GetHistogram();
  hists.saveLab("inthist_lab.txt");
  hists.saveCMS("inthist_cms.txt");

  std::ofstream finish("finished");
  finish << "run completed without error" << std::endl;
}
