/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/cascade/Cascade.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/process/particle_cut/ParticleCut.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <corsika/utl/CorsikaFenv.h>

#include <corsika/logging/Logging.h>

#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::particles;
using namespace corsika::random;
using namespace corsika::geometry;
using namespace corsika::environment;

using namespace std;
using namespace corsika::units::si;

template <bool deleteParticle>
struct MyBoundaryCrossingProcess
    : public BoundaryCrossingProcess<MyBoundaryCrossingProcess<deleteParticle>> {

  MyBoundaryCrossingProcess(std::string const& filename) { fFile.open(filename); }

  template <typename Particle>
  EProcessReturn DoBoundaryCrossing(Particle& p,
                                    typename Particle::BaseNodeType const& from,
                                    typename Particle::BaseNodeType const& to) {

    C8LOG_INFO("MyBoundaryCrossingProcess: crossing! from: {} to: {} ", fmt::ptr(&from),
               fmt::ptr(&to));

    auto const& name = particles::GetName(p.GetPID());
    auto const start = p.GetPosition().GetCoordinates();

    fFile << name << "    " << start[0] / 1_m << ' ' << start[1] / 1_m << ' '
          << start[2] / 1_m << '\n';

    if constexpr (deleteParticle) { p.Delete(); }

    return EProcessReturn::eOk;
  }

private:
  std::ofstream fFile;
};

//
// The example main program for a particle cascade
//
int main() {

  logging::SetLevel(logging::level::info);

  C8LOG_INFO("boundary_example");

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // create "world" as infinite sphere filled with protons
  auto world = EnvType::CreateNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  auto const props =
      world->SetModelProperties<environment::HomogeneousMedium<setup::IEnvironmentModel>>(
          1_kg / (1_m * 1_m * 1_m),
          environment::NuclearComposition(
              std::vector<particles::Code>{particles::Code::Proton},
              std::vector<float>{1.f}));

  // add a "target" sphere with 5km readius at 0,0,0
  auto target = EnvType::CreateNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5_km);
  target->SetModelProperties(props);

  world->AddChild(std::move(target));
  universe.AddChild(std::move(world));

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;

  random::RNGManager::GetInstance().RegisterRandomStream("sibyll");
  process::sibyll::Interaction sibyll;
  process::sibyll::Decay decay;

  process::particle_cut::ParticleCut cut(50_GeV, true, true);

  process::track_writer::TrackWriter trackWriter("boundary_tracks.dat");
  MyBoundaryCrossingProcess<true> boundaryCrossing("crossings.dat");

  // assemble all processes into an ordered process list
  auto sequence = process::sequence(sibyll, decay, cut, boundaryCrossing, trackWriter);

  // setup particle stack, and add primary particles
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::MuPlus;
  const HEPMassType mass = particles::GetMass(beamCode);
  const HEPEnergyType E0 = 100_GeV;

  std::uniform_real_distribution distTheta(0., 180.);
  std::uniform_real_distribution distPhi(0., 360.);
  std::mt19937 rng;

  for (int i = 0; i < 100; ++i) {
    double const theta = distTheta(rng);
    double const phi = distPhi(rng);

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
    C8LOG_INFO(
        "input particle: {} "
        "input angles: theta={} phi={}"
        "input momentum: {} GeV",
        beamCode, theta, phi, plab.GetComponents() / 1_GeV);
    // shoot particles from inside target out
    Point pos(rootCS, 0_m, 0_m, 0_m);
    stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            beamCode, E0, plab, pos, 0_ns});
  }

  // define air shower object, run simulation
  cascade::Cascade EAS(env, tracking, sequence, stack);

  EAS.Run();

  C8LOG_INFO("Result: E0={}GeV", E0 / 1_GeV);
  cut.ShowResults();
  [[maybe_unused]] const HEPEnergyType Efinal =
      (cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy());
  C8LOG_INFO("Total energy (GeV): {} relative difference (%): {}", Efinal / 1_GeV,
             (Efinal / E0 - 1.) * 100);
}
