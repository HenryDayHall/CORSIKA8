/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/StackProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/SwitchProcess.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/TrackingLine.hpp>
#include <corsika/modules/UrQMD.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace corsika::setup;
using namespace std;

void registerRandomStreams() {
  corsika::RNGManager::getInstance().registerRandomStream("cascade");
  corsika::RNGManager::getInstance().registerRandomStream("s_rndm");
  // corsika::RNGManager::getInstance().registerRandomStream("pythia");
  corsika::RNGManager::getInstance().registerRandomStream("UrQMD");

  corsika::RNGManager::getInstance().seedAll();
}

int main() {
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  registerRandomStreams();

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  corsika::LayeredSphericalAtmosphereBuilder builder(Point{rootCS, 0_m, 0_m, 0_m});
  builder.setNuclearComposition(
      {{corsika::Code::Nitrogen, corsika::Code::Oxygen},
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
  const Code beamCode = Code::Proton;
  auto const mass = corsika::GetMass(beamCode);
  const HEPEnergyType E0 = 0.1_PeV;
  double theta = 0.;
  double phi = 0.;

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      112.8_km * 0.999 +
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
  auto plab = corsika::MomentumVector(rootCS, {px, py, pz});
  std::cout << "input particle: " << beamCode << std::endl;
  std::cout << "input angles: theta=" << theta << " phi=" << phi << std::endl;
  std::cout << "input momentum: " << plab.GetComponents() / 1_GeV << std::endl;

  stack.AddParticle(
      std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, corsika::Point,
                 TimeType>{beamCode, E0, plab, injectionPos, 0_ns});
  //  }

  Line const line(injectionPos, plab.normalized() * 1_m * 1_Hz);
  auto const velocity = line.GetV0().norm();

  auto const observationHeight = 1.425_km + builder.earthRadius;

  setup::Trajectory const showerAxis(line, (112.8_km - observationHeight) / velocity);

  // setup processes, decays and interactions

  corsika::sibyll::Interaction sibyll;
  corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  corsika::sibyll::Decay decay;

  corsika::particle_cut::ParticleCut cut(5_GeV);

  corsika::track_writer::TrackWriter trackWriter("tracks.dat");
  corsika::energy_loss::BetheBlochPDG eLoss(showerAxis);

  Plane const obsPlane(Point(rootCS, 0_m, 0_m, observationHeight),
                       Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
  corsika::observation_plane::ObservationPlane observationLevel(obsPlane,
                                                                "particles.dat");

  // assemble all processes into an ordered process list

  corsika::urqmd::UrQMD urqmd;

  auto sibyllSequence = sibyll << sibyllNuc;
  corsika::switch_process::SwitchProcess switchProcess(urqmd, sibyllSequence, 55_GeV);
  auto sequence = switchProcess << decay << eLoss << cut << observationLevel
                                << trackWriter;

  // define air shower object, run simulation
  tracking_line::TrackingLine tracking;
  corsika::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  eLoss.PrintProfile(); // print longitudinal profile

  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  std::cout << "total cut energy (GeV): " << Efinal / 1_GeV << std::endl
            << "relative difference (%): " << (Efinal / E0 - 1) * 100 << std::endl;
  std::cout << "total dEdX energy (GeV): " << eLoss.GetTotal() / 1_GeV << std::endl
            << "relative difference (%): " << eLoss.GetTotal() / E0 * 100 << std::endl;

  std::ofstream finish("finished");
  finish << "run completed without error" << std::endl;
}
