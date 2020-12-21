/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/logging/Logging.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/MediumPropertyModel.hpp>

#include <corsika/modules/TrackingLine.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/ParticleCut.hpp>

#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace std;

template <bool deleteParticle>
struct MyBoundaryCrossingProcess
    : public BoundaryCrossingProcess<MyBoundaryCrossingProcess<deleteParticle>> {

  MyBoundaryCrossingProcess(std::string const& filename) { file_.open(filename); }

  template <typename Particle>
  ProcessReturn doBoundaryCrossing(Particle& p, typename Particle::node_type const& from,
                                   typename Particle::node_type const& to) {

    CORSIKA_LOG_INFO("MyBoundaryCrossingProcess: crossing! from: {} to: {} ",
                     fmt::ptr(&from), fmt::ptr(&to));

    auto const& name = get_name(p.getPID());
    auto const start = p.getPosition().getCoordinates();

    file_ << name << "    " << start[0] / 1_m << ' ' << start[1] / 1_m << ' '
          << start[2] / 1_m << '\n';

    if constexpr (deleteParticle) { p.erase(); }

    return ProcessReturn::Ok;
  }

private:
  std::ofstream file_;
};

//
// The example main program for a particle cascade
//
int main() {

  // logging::SetLevel(logging::level::info);

  CORSIKA_LOG_INFO("boundary_example");

  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  RNGManager::getInstance().registerRandomStream("cascade");

  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  auto& universe = *(env.getUniverse());

  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  // create "world" as infinite sphere filled with protons
  auto world = EnvType::createNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

  auto const props = world->setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, Vector(rootCS, 0_T, 0_T, 0_T), 1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(std::vector<Code>{Code::Proton}, std::vector<float>{1.f}));

  // add a "target" sphere with 5km readius at 0,0,0
  auto target = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5_km);
  target->setModelProperties(props);

  world->addChild(std::move(target));
  universe.addChild(std::move(world));

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;

  RNGManager::getInstance().registerRandomStream("sibyll");
  corsika::sibyll::Interaction sibyll;
  corsika::sibyll::Decay decay;

  particle_cut::ParticleCut cut(50_GeV, true, true);

  track_writer::TrackWriter trackWriter("boundary_tracks.dat");
  MyBoundaryCrossingProcess<true> boundaryCrossing("crossings.dat");

  // assemble all processes into an ordered process list
  auto sequence = make_sequence(sibyll, decay, cut, boundaryCrossing, trackWriter);

  // setup particle stack, and add primary particles
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::MuPlus;
  const HEPMassType mass = get_mass(beamCode);
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
    auto plab = MomentumVector(rootCS, {px, py, pz});
    CORSIKA_LOG_INFO(
        "input particle: {} "
        "input angles: theta={} phi={}"
        "input momentum: {} GeV",
        beamCode, theta, phi, plab.getComponents() / 1_GeV);
    // shoot particles from inside target out
    Point pos(rootCS, 0_m, 0_m, 0_m);
    stack.addParticle(std::make_tuple(beamCode, E0, plab, pos, 0_ns));
  }

  // define air shower object, run simulation
  Cascade EAS(env, tracking, sequence, stack);

  EAS.run();

  CORSIKA_LOG_INFO("Result: E0={}GeV", E0 / 1_GeV);
  cut.showResults();
  [[maybe_unused]] const HEPEnergyType Efinal =
      (cut.getCutEnergy() + cut.getInvEnergy() + cut.getEmEnergy());
  CORSIKA_LOG_INFO("Total energy (GeV): {} relative difference (%): {}", Efinal / 1_GeV,
                   (Efinal / E0 - 1.) * 100);
}
