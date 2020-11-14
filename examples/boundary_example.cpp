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
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/TrackingLine.hpp>

#include <iostream>
#include <limits>
#include <typeinfo>

using namespace corsika;
using namespace corsika::units::si;
using namespace std;

template <bool deleteParticle>
struct MyBoundaryCrossingProcess
    : public BoundaryCrossingProcess<MyBoundaryCrossingProcess<deleteParticle>> {

  MyBoundaryCrossingProcess(std::string const& filename) { fFile.open(filename); }

  template <typename Particle>
  EProcessReturn DoBoundaryCrossing(Particle& p,
                                    typename Particle::BaseNodeType const& from,
                                    typename Particle::BaseNodeType const& to) {
    std::cout << "boundary crossing! from: " << &from << "; to: " << &to << std::endl;

    auto const& name = corsika::GetName(p.GetPID());
    auto const start = p.GetPosition().GetCoordinates();

    fFile << name << "    " << start[0] / 1_m << ' ' << start[1] / 1_m << ' '
          << start[2] / 1_m << '\n';

    if constexpr (deleteParticle) { p.Delete(); }

    return EProcessReturn::eOk;
  }

  void Init() {}

private:
  std::ofstream fFile;
};

//
// The example main program for a particle cascade
//
int main() {
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  corsika::RNGManager::getInstance().registerRandomStream("cascade");

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  auto outerMedium = EnvType::CreateNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  auto const props =
      outerMedium
          ->SetModelProperties<corsika::HomogeneousMedium<setup::IEnvironmentModel>>(
              1_kg / (1_m * 1_m * 1_m),
              corsika::NuclearComposition(
                  std::vector<corsika::Code>{corsika::Code::Proton},
                  std::vector<float>{1.f}));

  auto innerMedium = EnvType::CreateNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5_km);

  innerMedium->SetModelProperties(props);

  outerMedium->AddChild(std::move(innerMedium));

  universe.AddChild(std::move(outerMedium));

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;

  RNGManager::getInstance().registerRandomStream("s_rndm");
  corsika::sibyll::Interaction sibyll;
  corsika::sibyll::Decay decay;

  corsika::particle_cut::ParticleCut cut(20_GeV);

  corsika::track_writer::TrackWriter trackWriter("tracks.dat");
  MyBoundaryCrossingProcess<true> boundaryCrossing("crossings.dat");

  // assemble all processes into an ordered process list
  auto sequence = sibyll << decay << cut << boundaryCrossing << trackWriter;

  // setup particle stack, and add primary particles
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Proton;
  const HEPMassType mass = corsika::GetMass(Code::Proton);
  const HEPEnergyType E0 = 50_TeV;

  std::uniform_real_distribution distTheta(0., 180.);
  std::uniform_real_distribution distPhi(0., 360.);
  std::mt19937 rng;

  for (int i = 0; i < 100; ++i) {
    auto const theta = distTheta(rng);
    auto const phi = distPhi(rng);

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
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{beamCode, E0, plab, pos, 0_ns});
  }

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
