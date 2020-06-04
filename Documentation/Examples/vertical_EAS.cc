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
#include <corsika/environment/Environment.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/ShowerAxis.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/StackProcess.h>
#include <corsika/process/energy_loss/EnergyLoss.h>
#include <corsika/process/interaction_counter/InteractionCounter.h>
#include <corsika/process/longitudinal_profile/LongitudinalProfile.h>
#include <corsika/process/observation_plane/ObservationPlane.h>
#include <corsika/process/on_shell_check/OnShellCheck.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/pythia/Decay.h>
#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>
#include <corsika/process/switch_process/SwitchProcess.h>
#include <corsika/process/tracking_line/TrackingLine.h>
#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/random/RNGManager.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
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
    std::cerr << "usage: vertical_EAS <A> <Z> <energy/GeV>" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  registerRandomStreams();

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  environment::LayeredSphericalAtmosphereBuilder builder{center};
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
  auto const thetaRad = theta / 180. * M_PI;

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };
  HEPMomentumType P0 = elab2plab(E0, mass);
  auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
  };

  auto const [px, py, pz] = momentumComponents(thetaRad, P0);
  auto plab = corsika::stack::MomentumVector(rootCS, {px, py, pz});
  cout << "input particle: " << beamCode << endl;
  cout << "input angles: theta=" << theta << endl;
  cout << "input momentum: " << plab.GetComponents() / 1_GeV << ", norm = " << plab.norm()
       << endl;

  auto const observationHeight = 1.4_km + builder.getEarthRadius();
  auto const injectionHeight = 112.75_km + builder.getEarthRadius();
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-si::detail::static_pow<2>(sin(thetaRad) * observationHeight) +
                      si::detail::static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore +
      Vector<dimensionless_d>{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  std::cout << "point of injection: " << injectionPos.GetCoordinates() << std::endl;

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

  std::cout << "shower axis length: " << (showerCore - injectionPos).norm() * 1.02
            << std::endl;

  environment::ShowerAxis const showerAxis{injectionPos,
                                           (showerCore - injectionPos) * 1.02, env};

  // setup processes, decays and interactions

  process::sibyll::Interaction sibyll;
  process::interaction_counter::InteractionCounter sibyllCounted(sibyll);

  process::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  process::interaction_counter::InteractionCounter sibyllNucCounted(sibyllNuc);

  process::pythia::Decay decayPythia;

  // use sibyll decay routine for decays of particles unknown to pythia
  process::sibyll::Decay decaySibyll{{
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
  }};

  decaySibyll.PrintDecayConfig();

  process::particle_cut::ParticleCut cut{60_GeV};

  process::on_shell_check::OnShellCheck reset_particle_mass(1.e-3, 1.e-2);

  process::energy_loss::EnergyLoss eLoss(showerAxis);
  process::longitudinal_profile::LongitudinalProfile longprof{showerAxis};

  Plane const obsPlane(showerCore, Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
  process::observation_plane::ObservationPlane observationLevel(obsPlane,
                                                                "particles.dat");

  // assemble all processes into an ordered process list

  process::UrQMD::UrQMD urqmd;
  process::interaction_counter::InteractionCounter urqmdCounted{urqmd};

  auto sibyllSequence = sibyllNucCounted << sibyllCounted;
  process::switch_process::SwitchProcess switchProcess(urqmdCounted, sibyllSequence,
                                                       55_GeV);
  auto decaySequence = decayPythia << decaySibyll;

  auto sequence = switchProcess << reset_particle_mass << decaySequence << longprof << eLoss << cut
                                << observationLevel;

  // define air shower object, run simulation
  tracking_line::TrackingLine tracking;
  cascade::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();

  // to fix the point of first interaction, uncomment the following two lines:
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

  auto const hists = sibyllCounted.GetHistogram() + sibyllNucCounted.GetHistogram() +
                     urqmdCounted.GetHistogram();
  hists.saveLab("inthist_lab.txt");
  hists.saveCMS("inthist_cms.txt");

  longprof.save("longprof.txt");

  std::ofstream finish("finished");
  finish << "run completed without error" << std::endl;
}
