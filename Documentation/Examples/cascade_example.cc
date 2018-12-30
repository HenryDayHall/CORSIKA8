/**
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
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;

#include <iostream>
#include <limits>
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
using namespace corsika::units::hep;

class ProcessCut : public corsika::process::ContinuousProcess<ProcessCut> {

  EnergyType fECut;

  EnergyType fEnergy = 0_GeV;
  EnergyType fEmEnergy = 0_GeV;
  int fEmCount = 0;
  EnergyType fInvEnergy = 0_GeV;
  int fInvCount = 0;

public:
  ProcessCut(const EnergyType v)
      : fECut(v) {}
  template <typename Particle>
  bool isBelowEnergyCut(Particle& p) const {
    // FOR NOW: center-of-mass energy hard coded
    const EnergyType Ecm = sqrt(2. * p.GetEnergy() * 0.93827_GeV);
    if (p.GetEnergy() < fECut || Ecm < 10_GeV)
      return true;
    else
      return false;
  }

  bool isEmParticle(Code pCode) const {
    bool is_em = false;
    // FOR NOW: switch
    switch (pCode) {
      case Code::Electron:
        is_em = true;
        break;
      case Code::Positron:
        is_em = true;
        break;
      case Code::Gamma:
        is_em = true;
        break;
      default:
        break;
    }
    return is_em;
  }

  void defineEmParticles() const {
    // create bool array identifying em particles
  }

  bool isInvisible(Code pCode) const {
    bool is_inv = false;
    // FOR NOW: switch
    switch (pCode) {
      case Code::NuE:
        is_inv = true;
        break;
      case Code::NuEBar:
        is_inv = true;
        break;
      case Code::NuMu:
        is_inv = true;
        break;
      case Code::NuMuBar:
        is_inv = true;
        break;
      case Code::MuPlus:
        is_inv = true;
        break;
      case Code::MuMinus:
        is_inv = true;
        break;

      default:
        break;
    }
    return is_inv;
  }

  template <typename Particle>
  LengthType MaxStepLength(Particle& p, setup::Trajectory&) const {
    cout << "ProcessCut: MinStep: pid: " << p.GetPID() << endl;
    cout << "ProcessCut: MinStep: energy (GeV): " << p.GetEnergy() / 1_GeV << endl;
    const Code pid = p.GetPID();
    if (isEmParticle(pid) || isInvisible(pid) || isBelowEnergyCut(p)) {
      cout << "ProcessCut: MinStep: next cut: " << 0. << endl;
      return 0_m;
    } else {
      LengthType next_step = 1_m * std::numeric_limits<double>::infinity();
      cout << "ProcessCut: MinStep: next cut: " << next_step << endl;
      return next_step;
    }
  }

  template <typename Particle, typename Stack>
  EProcessReturn DoContinuous(Particle& p, setup::Trajectory&, Stack&) {
    const Code pid = p.GetPID();
    EnergyType energy = p.GetEnergy();
    cout << "ProcessCut: DoContinuous: " << pid << " E= " << energy
         << ", EcutTot=" << (fEmEnergy + fInvEnergy + fEnergy) / 1_GeV << " GeV" << endl;
    EProcessReturn ret = EProcessReturn::eOk;
    if (isEmParticle(pid)) {
      cout << "removing em. particle..." << endl;
      fEmEnergy += energy;
      fEmCount += 1;
      // p.Delete();
      ret = EProcessReturn::eParticleAbsorbed;
    } else if (isInvisible(pid)) {
      cout << "removing inv. particle..." << endl;
      fInvEnergy += energy;
      fInvCount += 1;
      // p.Delete();
      ret = EProcessReturn::eParticleAbsorbed;
    } else if (isBelowEnergyCut(p)) {
      cout << "removing low en. particle..." << endl;
      fEnergy += energy;
      // p.Delete();
      ret = EProcessReturn::eParticleAbsorbed;
    }
    return ret;
  }

  void Init() {
    fEmEnergy = 0. * 1_GeV;
    fEmCount = 0;
    fInvEnergy = 0. * 1_GeV;
    fInvCount = 0;
    fEnergy = 0. * 1_GeV;
    // defineEmParticles();
  }

  void ShowResults() {
    cout << " ******************************" << endl
         << " ParticleCut: " << endl
         << " energy in em.  component (GeV):  " << fEmEnergy / 1_GeV << endl
         << " no. of em.  particles injected:  " << fEmCount << endl
         << " energy in inv. component (GeV):  " << fInvEnergy / 1_GeV << endl
         << " no. of inv. particles injected:  " << fInvCount << endl
         << " energy below particle cut (GeV): " << fEnergy / 1_GeV << endl
         << " ******************************" << endl;
  }

  EnergyType GetInvEnergy() const { return fInvEnergy; }
  EnergyType GetCutEnergy() const { return fEnergy; }
  EnergyType GetEmEnergy() const { return fEmEnergy; }
};

//
// The example main program for a particle cascade
//
int main() {
  // initialize random number sequence(s)
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  corsika::environment::Environment env;
  auto& universe = *(env.GetUniverse());

  auto theMedium = corsika::environment::Environment::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  // fraction of oxygen
  const float fox = 0.20946;
  using MyHomogeneousModel =
      corsika::environment::HomogeneousMedium<corsika::environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      corsika::environment::NuclearComposition(
					       std::vector<corsika::particles::Code>{corsika::particles::Code::Nitrogen,corsika::particles::Code::Oxygen},
					       std::vector<float>{(float)1.-fox, fox}));

  universe.AddChild(std::move(theMedium));

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup processes, decays and interactions
  tracking_line::TrackingLine<setup::Stack> tracking(env);
  stack_inspector::StackInspector<setup::Stack> p0(true);
  corsika::process::sibyll::Interaction sibyll(env);
  corsika::process::sibyll::Decay decay;
  ProcessCut cut(8_GeV);
  corsika::process::TrackWriter::TrackWriter trackWriter("tracks.dat");

  // assemble all processes into an ordered process list
  auto sequence = p0 << sibyll << decay << cut << trackWriter;

  // cout << "decltype(sequence)=" << type_id_with_cvr<decltype(sequence)>().pretty_name()
  // << "\n";

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const hep::EnergyType E0 = 100_GeV;
  double theta = 0.;
  double phi = 0.;
  {
    auto particle = stack.NewParticle();
    particle.SetPID(Code::Proton);
    hep::MomentumType P0 = sqrt(E0 * E0 - Proton::GetMass() * Proton::GetMass());
    auto momentumComponents = [](double theta, double phi, MomentumType& ptot) {
      return std::make_tuple(ptot * sin(theta) * cos(phi), ptot * sin(theta) * sin(phi),
                             -ptot * cos(theta));
    };
    auto const [px, py, pz] =
        momentumComponents(theta / 180. * M_PI, phi / 180. * M_PI, P0);
    auto plab = stack::super_stupid::MomentumVector(rootCS, {px, py, pz});
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    particle.SetEnergy(E0);
    particle.SetMomentum(plab);
    particle.SetTime(0_ns);
    Point p(rootCS, 0_m, 0_m, 0_m);
    particle.SetPosition(p);
  }

  // define air shower object, run simulation
  corsika::cascade::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  cout << "Result: E0=" << E0 / 1_GeV << endl;
  cut.ShowResults();
  cout << "total energy (GeV): "
       << (cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy()) / 1_GeV << endl;
}
