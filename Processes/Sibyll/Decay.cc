
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/Decay.h>

#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using std::cout;
using std::endl;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika::setup;
using Particle = Stack::StackIterator; // ParticleType;
using Track = Trajectory;

namespace corsika::process::sibyll {

  Decay::Decay() {}
  Decay::~Decay() { cout << "Sibyll::Decay n=" << fCount << endl; }
  void Decay::Init() {
    setHadronsUnstable();
    setTrackedParticlesStable();
  }

  void Decay::setTrackedParticlesStable() {
    /*
       Sibyll is hadronic generator
       only hadrons decay
     */
    // set particles unstable
    setHadronsUnstable();
    // make tracked particles stable
    cout << "Interaction: setting tracked hadrons stable.." << endl;
    const vector<particles::Code> particleList = {
        particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::KMinus, particles::Code::K0Long,  particles::Code::K0Short};

    for (auto p : particleList) {
      // set particle stable by setting table value negative
      const int sibid = process::sibyll::ConvertToSibyllRaw(p);
      s_csydec_.idb[sibid - 1] = (-1) * abs(s_csydec_.idb[sibid - 1]);
    }
  }

  void Decay::setUnstable(const particles::Code pCode) {
    int s_id = process::sibyll::ConvertToSibyllRaw(pCode);
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::setStable(const particles::Code pCode) {
    int s_id = process::sibyll::ConvertToSibyllRaw(pCode);
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::setAllStable() {
    // name? also makes EM particles stable

    cout << "Decay: setting all particles stable.." << endl;

    // loop over all particles in sibyll
    // should be changed to loop over human readable list
    // i.e. particles::ListOfParticles()
    for (auto& p : corsika2sibyll) {
      // cout << (int)p << endl;
      const int sibCode = static_cast<int>(p);
      // skip unknown and antiparticles
      if (sibCode < 1) continue;
      s_csydec_.idb[sibCode - 1] = -1 * abs(s_csydec_.idb[sibCode - 1]);
    }
  }

  void Decay::setHadronsUnstable() {

    // name? also makes EM particles stable

    // loop over all particles in sibyll
    // should be changed to loop over human readable list
    // i.e. particles::ListOfParticles()
    cout << "Sibyll: setting hadrons unstable.." << endl;
    // make ALL particles unstable, then set EM stable
    for (int sibCode : corsika2sibyll) {
      if (sibCode < 1) continue;
      s_csydec_.idb[sibCode - 1] = abs(s_csydec_.idb[sibCode - 1]);
    }
    // set Leptons and Proton and Neutron stable
    // use stack to loop over particles
    constexpr particles::Code particleList[] = {
        particles::Code::Proton,   particles::Code::Neutron, particles::Code::Electron,
        particles::Code::Positron, particles::Code::NuE,     particles::Code::NuEBar,
        particles::Code::MuMinus,  particles::Code::MuPlus,  particles::Code::NuMu,
        particles::Code::NuMuBar};

    for (auto p : particleList) {
      const int sibid = process::sibyll::ConvertToSibyllRaw(p);
      s_csydec_.idb[sibid - 1] = (-1) * abs(s_csydec_.idb[sibid - 1]);
    }
  }

  template <>
  units::si::TimeType Decay::GetLifetime(Particle const& p) {
    using namespace units::si;

    HEPEnergyType E = p.GetEnergy();
    HEPMassType m = p.GetMass();

    const double gamma = E / m;

    const TimeType t0 = particles::GetLifetime(p.GetPID());
    auto const lifetime = gamma * t0;

    const auto mkin =
        (E * E - p.GetMomentum().squaredNorm()); // delta_mass(p.GetMomentum(), E, m);
    cout << "Decay: code: " << p.GetPID() << endl;
    cout << "Decay: MinStep: t0: " << t0 << endl;
    cout << "Decay: MinStep: energy: " << E / 1_GeV << " GeV" << endl;
    cout << "Decay: momentum: " << p.GetMomentum().GetComponents() / 1_GeV << " GeV"
         << endl;
    cout << "Decay: momentum: shell mass-kin. inv. mass " << mkin / 1_GeV / 1_GeV << " "
         << m / 1_GeV * m / 1_GeV << endl;
    auto sib_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
    cout << "Decay: sib mass: " << get_sibyll_mass2(sib_id) << endl;
    cout << "Decay: MinStep: gamma: " << gamma << endl;
    cout << "Decay: MinStep: tau: " << lifetime << endl;

    return lifetime;
  }

  template <>
  void Decay::DoDecay(Particle& p, Stack&) {
    using geometry::Point;
    using namespace units::si;

    fCount++;
    SibStack ss;
    ss.Clear();
    const particles::Code pCode = p.GetPID();
    // copy particle to sibyll stack
    ss.AddParticle(process::sibyll::ConvertToSibyllRaw(pCode), p.GetEnergy(),
                   p.GetMomentum(),
                   // setting particle mass with Corsika values, may be inconsistent
                   // with sibyll internal values
                   particles::GetMass(pCode));
    // remember position
    Point const decayPoint = p.GetPosition();
    TimeType const t0 = p.GetTime();
    // remove original particle from corsika stack
    p.Delete();
    // set all particles/hadrons unstable
    // setHadronsUnstable();
    setUnstable(pCode);
    // call sibyll decay
    cout << "Decay: calling Sibyll decay routine.." << endl;
    decsib_();
    // reset to stable
    setStable(pCode);
    // print output
    int print_unit = 6;
    sib_list_(print_unit);

    // copy particles from sibyll stack to corsika
    for (auto& psib : ss) {
      // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
      if (psib.HasDecayed()) continue;
      // add to corsika stack
      p.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                geometry::Point, units::si::TimeType>{
              process::sibyll::ConvertFromSibyll(psib.GetPID()), psib.GetEnergy(),
              psib.GetMomentum(), decayPoint, t0});
    }
    // empty sibyll stack
    ss.Clear();
  }

} // namespace corsika::process::sibyll
