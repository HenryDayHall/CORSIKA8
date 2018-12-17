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

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

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
using namespace corsika::units::si;

static EnergyType fEnergy = 0. * 1_GeV;

// FOR NOW: global static variables for ParticleCut process
// this is just wrong...
static EnergyType fEmEnergy;
static int fEmCount;

static EnergyType fInvEnergy;
static int fInvCount;

class ProcessEMCut : public corsika::process::BaseProcess<ProcessEMCut> {
public:
  ProcessEMCut() {}
  template <typename Particle>
  bool isBelowEnergyCut(Particle& p) const {
    // FOR NOW: center-of-mass energy hard coded
    const EnergyType Ecm = sqrt(2. * p.GetEnergy() * 0.93827_GeV);
    if (p.GetEnergy() < 50_GeV || Ecm < 10_GeV)
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
  double MaxStepLength(Particle& p, setup::Trajectory&) const {
    const Code pid = p.GetPID();
    if (isEmParticle(pid) || isInvisible(pid)) {
      cout << "ProcessCut: MinStep: next cut: " << 0. << endl;
      return 0.;
    } else {
      double next_step = std::numeric_limits<double>::infinity();
      cout << "ProcessCut: MinStep: next cut: " << next_step << endl;
      return next_step;
    }
  }

  template <typename Particle, typename Stack>
  EProcessReturn DoContinuous(Particle&, setup::Trajectory&, Stack&) const {
    // cout << "ProcessCut: DoContinous: " << p.GetPID() << endl;
    // cout << " is em: " << isEmParticle( p.GetPID() ) << endl;
    // cout << " is inv: " << isInvisible( p.GetPID() ) << endl;
    // const Code pid = p.GetPID();
    // if( isEmParticle( pid ) ){
    //   cout << "removing em. particle..." << endl;
    //   fEmEnergy += p.GetEnergy();
    //   fEmCount  += 1;
    //   p.Delete();
    //   return EProcessReturn::eParticleAbsorbed;
    // }
    // if ( isInvisible( pid ) ){
    //   cout << "removing inv. particle..." << endl;
    //   fInvEnergy += p.GetEnergy();
    //   fInvCount  += 1;
    //   p.Delete();
    //   return EProcessReturn::eParticleAbsorbed;
    // }
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack&) const {
    cout << "ProcessCut: DoDiscrete: " << p.GetPID() << endl;
    const Code pid = p.GetPID();
    if (isEmParticle(pid)) {
      cout << "removing em. particle..." << endl;
      fEmEnergy += p.GetEnergy();
      fEmCount += 1;
      p.Delete();
    } else if (isInvisible(pid)) {
      cout << "removing inv. particle..." << endl;
      fInvEnergy += p.GetEnergy();
      fInvCount += 1;
      p.Delete();
    } else if (isBelowEnergyCut(p)) {
      cout << "removing low en. particle..." << endl;
      fEnergy += p.GetEnergy();
      p.Delete();
    }
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
         << " energy in em.  component (GeV): " << fEmEnergy / 1_GeV << endl
         << " no. of em.  particles injected: " << fEmCount << endl
         << " energy in inv. component (GeV): " << fInvEnergy / 1_GeV << endl
         << " no. of inv. particles injected: " << fInvCount << endl
         << " ******************************" << endl;
  }

  EnergyType GetInvEnergy() { return fInvEnergy; }

  EnergyType GetCutEnergy() { return fEnergy; }

  EnergyType GetEmEnergy() { return fEmEnergy; }

private:
};

int main() {
    
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("cascade");
    
  corsika::environment::Environment env; // dummy environment
  auto& universe = *(env.GetUniverse());

  auto theMedium = corsika::environment::Environment::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel =
      corsika::environment::HomogeneousMedium<corsika::environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_g / (1_m * 1_m * 1_m),
      corsika::environment::NuclearComposition(
          std::vector<corsika::particles::Code>{corsika::particles::Code::Proton},
          std::vector<float>{1.}));

  universe.AddChild(std::move(theMedium));

  CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

  tracking_line::TrackingLine<setup::Stack> tracking(env);
  stack_inspector::StackInspector<setup::Stack> p0(true);

  corsika::process::sibyll::Interaction /*<setup::Stack>,setup::Trajectory>*/ p1;
  corsika::process::sibyll::Decay p2;
  ProcessEMCut p3;
  const auto sequence = /*p0 +*/ p1 + p2 + p3;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(env, tracking, sequence, stack);

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  MomentumType P0 = sqrt(E0 * E0 - 0.93827_GeV * 0.93827_GeV) / si::constants::c;
  auto plab = super_stupid::MomentumVector(rootCS, 0. * 1_GeV / si::constants::c,
                                           0. * 1_GeV / si::constants::c, P0);
  particle.SetEnergy(E0);
  particle.SetMomentum(plab);
  particle.SetPID(Code::Proton);
  particle.SetTime(0_ns);
  Point p(rootCS, 0_m, 0_m, 0_m);
  particle.SetPosition(p);
  EAS.Init();
  EAS.Run();
  cout << "Result: E0="
       << E0 / 1_GeV
       //<< "GeV, particles below energy threshold =" << p1.GetCount()
       << endl;
  cout << "total energy below threshold (GeV): " //<< p1.GetEnergy() / 1_GeV
       << std::endl;
  p3.ShowResults();
  cout << "total energy (GeV): "
       << (p3.GetCutEnergy() + p3.GetInvEnergy() + p3.GetEmEnergy()) / 1_GeV << endl;
}
