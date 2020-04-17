/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/sequence/ProcessSequence.hpp>
#include <corsika/process/tracking_line/TrackingLine.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/framework/geometry/Sphere.hpp>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/process/particle_cut/ParticleCut.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/logging/Logging.h>

#include <iostream>
#include <limits>
#include <typeinfo>

#include "../../corsika/setup/SetupEnvironment.hpp"
#include "../../corsika/setup/SetupStack.hpp"
#include "../../corsika/setup/SetupTrajectory.hpp"

using namespace corsika;
using namespace corsika;
using namespace corsika::units;
using namespace corsika;
using namespace corsika;
using namespace corsika;
using namespace corsika;
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

  logging::SetLevel(logging::level::trace);

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
      Point{rootCS, 0_m, 0_m, 0_m}, 100_km);

  using MyHomogeneousModel =
      environment::MediumPropertyModel<environment::UniformMagneticField<
          environment::HomogeneousMedium<setup::EnvironmentInterface>>>;

  auto const props = world->SetModelProperties<MyHomogeneousModel>(
      environment::Medium::AirDry1Atm, Vector(rootCS, 0_T, 0_T, 0_T),
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
  setup::Tracking tracking;

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
    auto plab = corsika::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    Point pos(rootCS, 0_m, 0_m, 0_m);
    stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::MomentumVector, geometry::Point, units::si::TimeType>{
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
