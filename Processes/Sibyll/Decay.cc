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

using SetupView = corsika::setup::StackView;
using SetupProjectile = corsika::setup::StackView::ParticleType;
using SetupParticle = corsika::setup::Stack::ParticleType;

namespace corsika::process::sibyll {

  Decay::Decay() {}
  Decay::~Decay() { cout << "Sibyll::Decay n=" << fCount << endl; }
  void Decay::Init() {
    // switch off decays to avoid internal decay chains
    SetAllStable();
  }

  void Decay::SetStable(const vector<particles::Code> vParticleList) {
    for (auto p : vParticleList) Decay::SetStable(p);
  }

  void Decay::SetUnstable(const vector<particles::Code> vParticleList) {
    for (auto p : vParticleList) Decay::SetUnstable(p);
  }

  bool Decay::IsStable(const particles::Code vCode) {
    return abs(process::sibyll::ConvertToSibyllRaw(vCode)) <= 0 ? true : false;
  }

  bool Decay::IsUnstable(const particles::Code vCode) {
    return abs(process::sibyll::ConvertToSibyllRaw(vCode)) > 0 ? true : false;
  }

  void Decay::SetDecay(const particles::Code vCode, const bool vMakeUnstable) {
    vMakeUnstable ? SetUnstable(vCode) : SetStable(vCode);
  }

  void Decay::SetUnstable(const particles::Code vCode) {
    cout << "Sibyll::Interaction: setting " << vCode << " unstable.." << endl;
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetStable(const particles::Code vCode) {
    cout << "Sibyll::Interaction: setting " << vCode << " stable.." << endl;
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
  }

  void Decay::SetAllUnstable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = abs(s_csydec_.idb[i]);
  }

  void Decay::PrintDecayConfig(const particles::Code vCode) {
    cout << "Decay: Sibyll decay configuration:" << endl;
    const int sibCode = process::sibyll::ConvertToSibyllRaw(vCode);
    const int absSibCode = abs(sibCode);
    cout << vCode << " is ";
    if (s_csydec_.idb[absSibCode - 1] <= 0)
      cout << "stable" << endl;
    else
      cout << "unstable" << endl;
  }

  template <>
  units::si::TimeType Decay::GetLifetime(SetupParticle const& vP) const {
    using namespace units::si;

    HEPEnergyType E = vP.GetEnergy();
    HEPMassType m = vP.GetMass();

    const double gamma = E / m;

    const TimeType t0 = particles::GetLifetime(vP.GetPID());
    auto const lifetime = gamma * t0;

    const auto mkin =
        (E * E - vP.GetMomentum().squaredNorm()); // delta_mass(vP.GetMomentum(), E, m);
    cout << "Decay: code: " << vP.GetPID() << endl;
    cout << "Decay: MinStep: t0: " << t0 << endl;
    cout << "Decay: MinStep: energy: " << E / 1_GeV << " GeV" << endl;
    cout << "Decay: momentum: " << vP.GetMomentum().GetComponents() / 1_GeV << " GeV"
         << endl;
    cout << "Decay: momentum: shell mass-kin. inv. mass " << mkin / 1_GeV / 1_GeV << " "
         << m / 1_GeV * m / 1_GeV << endl;
    auto sib_id = process::sibyll::ConvertToSibyllRaw(vP.GetPID());
    cout << "Decay: sib mass: " << get_sibyll_mass2(sib_id) << endl;
    cout << "Decay: MinStep: gamma: " << gamma << endl;
    cout << "Decay: MinStep: tau: " << lifetime << endl;

    return lifetime;
  }

  template <>
  void Decay::DoDecay(SetupProjectile& vP) {
    using geometry::Point;
    using namespace units::si;

    fCount++;
    SibStack ss;
    ss.Clear();
    const particles::Code pCode = vP.GetPID();
    // copy particle to sibyll stack
    ss.AddParticle(process::sibyll::ConvertToSibyllRaw(pCode), vP.GetEnergy(),
                   vP.GetMomentum(),
                   // setting particle mass with Corsika values, may be inconsistent
                   // with sibyll internal values
                   particles::GetMass(pCode));
    // remember position
    Point const decayPoint = vP.GetPosition();
    TimeType const t0 = vP.GetTime();
    // remember if particles is unstable
    // auto const priorIsUnstable = IsUnstable(pCode);
    // switch on decay for this particle
    SetUnstable(pCode);
    PrintDecayConfig(pCode);

    // call sibyll decay
    cout << "Decay: calling Sibyll decay routine.." << endl;
    decsib_();

    // reset to stable
    SetStable(pCode);
    // print output
    int print_unit = 6;
    sib_list_(print_unit);

    // copy particles from sibyll stack to corsika
    for (auto& psib : ss) {
      // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
      if (psib.HasDecayed()) continue;
      // add to corsika stack
      vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                geometry::Point, units::si::TimeType>{
              process::sibyll::ConvertFromSibyll(psib.GetPID()), psib.GetEnergy(),
              psib.GetMomentum(), decayPoint, t0});
    }
    // empty sibyll stack
    ss.Clear();
  }

} // namespace corsika::process::sibyll
