
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
#include <corsika/process/hadronic_elastic_model/HadronicElasticModel.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NameModel.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <corsika/utl/CorsikaFenv.h>

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

class ProcessCut : public process::ContinuousProcess<ProcessCut> {

  HEPEnergyType fECut;

  HEPEnergyType fEnergy = 0_GeV;
  HEPEnergyType fEmEnergy = 0_GeV;
  int fEmCount = 0;
  HEPEnergyType fInvEnergy = 0_GeV;
  int fInvCount = 0;

public:
  ProcessCut(const HEPEnergyType cut)
      : fECut(cut) {}

  template <typename Particle>
  bool isBelowEnergyCut(Particle& p) const {
    // nuclei
    if (p.GetPID() == particles::Code::Nucleus) {
      auto const ElabNuc = p.GetEnergy() / p.GetNuclearA();
      auto const EcmNN = sqrt(2. * ElabNuc * 0.93827_GeV);
      if (ElabNuc < fECut || EcmNN < 10_GeV)
        return true;
      else
        return false;
    } else {
      // TODO: center-of-mass energy hard coded
      const HEPEnergyType Ecm = sqrt(2. * p.GetEnergy() * 0.93827_GeV);
      if (p.GetEnergy() < fECut || Ecm < 10_GeV)
        return true;
      else
        return false;
    }
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

      case Code::Neutron:
        is_inv = true;
        break;

      case Code::AntiNeutron:
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
    HEPEnergyType energy = p.GetEnergy();
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

  HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
  HEPEnergyType GetCutEnergy() const { return fEnergy; }
  HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
};

template <bool deleteParticle>
struct MyBoundaryCrossingProcess
    : public BoundaryCrossingProcess<MyBoundaryCrossingProcess<deleteParticle>> {

  MyBoundaryCrossingProcess(std::string const& filename) { fFile.open(filename); }

  template <typename Particle>
  EProcessReturn DoBoundaryCrossing(Particle& p,
                                    typename Particle::BaseNodeType const& from,
                                    typename Particle::BaseNodeType const& to) {
    std::cout << "boundary crossing! from: " << &from << "; to: " << &to << std::endl;

    auto const& name = particles::GetName(p.GetPID());
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
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  using EnvType = environment::Environment<setup::IEnvironmentModel>;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  auto outerMedium = EnvType::CreateNode<Sphere>(
      Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  // fraction of oxygen
  const float fox = 0.20946;
  auto const props =
      outerMedium
          ->SetModelProperties<environment::HomogeneousMedium<setup::IEnvironmentModel>>(
              1_kg / (1_m * 1_m * 1_m),
              environment::NuclearComposition(
                  std::vector<particles::Code>{particles::Code::Nitrogen,
                                               particles::Code::Oxygen},
                  std::vector<float>{1.f - fox, fox}));

  auto innerMedium = EnvType::CreateNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 5000_m);

  innerMedium->SetModelProperties(props);

  outerMedium->AddChild(std::move(innerMedium));

  universe.AddChild(std::move(outerMedium));

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;
  stack_inspector::StackInspector<setup::Stack> p0(true);

  random::RNGManager::GetInstance().RegisterRandomStream("s_rndm");
  process::sibyll::Interaction sibyll;
  process::sibyll::NuclearInteraction sibyllNuc(sibyll);
  process::sibyll::Decay decay;
  ProcessCut cut(20_GeV);

  process::TrackWriter::TrackWriter trackWriter("tracks.dat");
  MyBoundaryCrossingProcess<false> boundaryCrossing("crossings.dat");

  // assemble all processes into an ordered process list
  auto sequence = sibyll << sibyllNuc << decay << cut << boundaryCrossing << trackWriter;

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Proton;
  const int nuclA = 56;
  const int nuclZ = int(nuclA / 2.15 + 0.7);
  const HEPMassType mass = particles::Proton::GetMass() * nuclZ +
                           (nuclA - nuclZ) * particles::Neutron::GetMass();
  const HEPEnergyType E0 = nuclA * 100_TeV;
  double const phi = 0;
  double const theta = 0;
  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt(Elab * Elab - m * m);
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
  Point pos(rootCS, 0_m, 0_m, 0_m);

  stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          beamCode, E0, plab, pos, 0_ns});

  // define air shower object, run simulation
  cascade::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  cout << "Result: E0=" << E0 / 1_GeV << endl;
  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1.) * 100 << endl;
}
