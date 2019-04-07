
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
#include <corsika/process/energy_loss/EnergyLoss.h>
#include <corsika/process/hadronic_elastic_model/HadronicElasticModel.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/geometry/Sphere.h>

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/process/pythia/Decay.h>

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/random/RNGManager.h>

#include <corsika/utl/CorsikaFenv.h>

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
using namespace corsika::units::si;

class ProcessCut : public process::SecondariesProcess<ProcessCut> {

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

  template <typename TSecondaries>
  EProcessReturn DoSecondaries(TSecondaries& vS) {
    auto p = vS.begin();
    while (p != vS.end()) {
      const Code pid = p.GetPID();
      HEPEnergyType energy = p.GetEnergy();
      cout << "ProcessCut: DoSecondaries: " << pid << " E= " << energy
           << ", EcutTot=" << (fEmEnergy + fInvEnergy + fEnergy) / 1_GeV << " GeV"
           << endl;
      if (isEmParticle(pid)) {
        cout << "removing em. particle..." << endl;
        fEmEnergy += energy;
        fEmCount += 1;
        p.Delete();
      } else if (isInvisible(pid)) {
        cout << "removing inv. particle..." << endl;
        fInvEnergy += energy;
        fInvCount += 1;
        p.Delete();
      } else if (isBelowEnergyCut(p)) {
        cout << "removing low en. particle..." << endl;
        fEnergy += energy;
        p.Delete();
      } else if (p.GetTime() > 10_ms) {
        cout << "removing OLD particle..." << endl;
        fEnergy += energy;
        p.Delete();
      } else {
        ++p; // next entry in SecondaryView
      }
    }
    return EProcessReturn::eOk;
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

class ObservationLevel : public process::ContinuousProcess<ObservationLevel> {

  LengthType fHeight;

public:
  ObservationLevel(const LengthType vHeight)
      : fHeight(vHeight) {}

  template <typename Particle>
  LengthType MaxStepLength(Particle&, setup::Trajectory&) const {
    return 1_m * std::numeric_limits<double>::infinity();
  }

  template <typename TParticle, typename TTrack>
  EProcessReturn DoContinuous(TParticle&, TTrack& vT) {
    if ((vT.GetPosition(0).GetZ() <= fHeight && vT.GetPosition(1).GetZ() > fHeight) ||
        (vT.GetPosition(0).GetZ() > fHeight && vT.GetPosition(1).GetZ() <= fHeight)) {
      cout << "OBSERVED " << endl;
      return EProcessReturn::eParticleAbsorbed;
    }
    return EProcessReturn::eOk;
  }
  void Init() {}
};

//
// The example main program for a particle cascade
//
int main() {
  feenableexcept(FE_INVALID);
  // initialize random number sequence(s)
  random::RNGManager::GetInstance().RegisterRandomStream("cascade");

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;
  auto& universe = *(env.GetUniverse());

  auto theMedium =
      EnvType::CreateNode<Sphere>(Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
                                  1_km * std::numeric_limits<double>::infinity());

  // fraction of oxygen
  const float fox = 0.20946;
  using MyHomogeneousModel = HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Nitrogen,
                                       particles::Code::Oxygen},
          std::vector<float>{(float)1. - fox, fox}));

  universe.AddChild(std::move(theMedium));

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup processes, decays and interactions
  tracking_line::TrackingLine tracking;

  const std::vector<particles::Code> trackedHadrons = {
      particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
      particles::Code::KMinus, particles::Code::K0Long,  particles::Code::K0Short};

  random::RNGManager::GetInstance().RegisterRandomStream("s_rndm");
  process::sibyll::Interaction sibyll;
  process::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  process::sibyll::Decay decay(trackedHadrons);
  // random::RNGManager::GetInstance().RegisterRandomStream("pythia");
  // process::pythia::Decay decay(trackedHadrons);
  ProcessCut cut(20_GeV);
  ObservationLevel obsLevel(1400_m);

  // random::RNGManager::GetInstance().RegisterRandomStream("HadronicElasticModel");
  // process::HadronicElasticModel::HadronicElasticInteraction
  // hadronicElastic(env);

  process::TrackWriter::TrackWriter trackWriter("tracks.dat");
  process::EnergyLoss::EnergyLoss eLoss;

  // assemble all processes into an ordered process list
  // cut << trackWriter;
  auto sequence = sibyll << sibyllNuc << decay << eLoss << cut << obsLevel;

  // cout << "decltype(sequence)=" << type_id_with_cvr<decltype(sequence)>().pretty_name()
  // << "\n";

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  const Code beamCode = Code::Nucleus;
  const int nuclA = 4;
  const int nuclZ = int(nuclA / 2.15 + 0.7);
  const HEPMassType mass = GetNucleusMass(nuclA, nuclZ);
  const HEPEnergyType E0 = nuclA * 10_TeV;
  double theta = 0.;
  double phi = 0.;

  {
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
    Point pos(rootCS, 0_m, 0_m,
              112.8_km); // this is the CORSIKA 7 start of atmosphere/universe
    stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
        beamCode, E0, plab, pos, 0_ns, nuclA, nuclZ});
  }

  // define air shower object, run simulation
  cascade::Cascade EAS(env, tracking, sequence, stack);
  EAS.Init();
  EAS.Run();

  eLoss.PrintProfile(); // print longitudinal profile

  cut.ShowResults();
  const HEPEnergyType Efinal =
      cut.GetCutEnergy() + cut.GetInvEnergy() + cut.GetEmEnergy();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  cout << "total dEdX energy (GeV): " << eLoss.GetTotal() / 1_GeV << endl
       << "relative difference (%): " << eLoss.GetTotal() / E0 * 100 << endl;
}
