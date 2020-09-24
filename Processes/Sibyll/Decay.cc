/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

  Decay::Decay() {
    // switch off decays to avoid internal decay chains
    SetAllStable();
    // handle all decays by default
    handleAllDecays_ = true;
  }

  Decay::Decay(std::set<particles::Code> vHandled)
      : handleAllDecays_(false)
      , handledDecays_(vHandled) {
    SetAllStable();
  }

  Decay::~Decay() { cout << "Sibyll::Decay n=" << fCount << endl; }

  bool Decay::CanHandleDecay(const particles::Code vParticleCode) {
    using namespace corsika::particles;
    // if known to sibyll and not proton or neutrino it can decay
    if (vParticleCode == Code::Proton || vParticleCode == Code::AntiProton ||
        vParticleCode == Code::NuE || vParticleCode == Code::NuMu ||
        vParticleCode == Code::NuTau || vParticleCode == Code::NuEBar ||
        vParticleCode == Code::NuMuBar || vParticleCode == Code::NuTauBar ||
        vParticleCode == Code::Electron || vParticleCode == Code::Positron)
      return false;
    else if (process::sibyll::ConvertToSibyllRaw(
                 vParticleCode)) // non-zero for particles known to sibyll
      return true;
    else
      return false;
  }

  void Decay::SetHandleDecay(const particles::Code vParticleCode) {
    handleAllDecays_ = false;
    cout << "Sibyll::Decay: set to handle decay of " << vParticleCode << endl;
    if (Decay::CanHandleDecay(vParticleCode))
      handledDecays_.insert(vParticleCode);
    else
      throw std::runtime_error("this decay can not be handled by sibyll!");
  }

  void Decay::SetHandleDecay(const vector<particles::Code> vParticleList) {
    handleAllDecays_ = false;
    for (auto p : vParticleList) Decay::SetHandleDecay(p);
  }

  bool Decay::IsDecayHandled(const corsika::particles::Code vParticleCode) {
    if (handleAllDecays_ && Decay::CanHandleDecay(vParticleCode))
      return true;
    else
      return Decay::handledDecays_.find(vParticleCode) != Decay::handledDecays_.end()
                 ? true
                 : false;
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
    cout << "Sibyll::Decay: setting " << vCode << " unstable.." << endl;
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetStable(const particles::Code vCode) {
    cout << "Sibyll::Decay: setting " << vCode << " stable.." << endl;
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

  void Decay::PrintDecayConfig() {
    cout << "Sibyll::Decay: decay configuration:" << endl;
    if (handleAllDecays_)
      cout << " all particles known to Sibyll are handled by Sibyll::Decay!" << endl;
    else
      for (auto& pCode : handledDecays_)
        cout << "Decay of " << pCode << " is handled by Sibyll!" << endl;
  }

  template <>
  units::si::TimeType Decay::GetLifetime(SetupParticle const& vP) {
    using namespace units::si;

    const particles::Code pid = vP.GetPID();
    if (Decay::IsDecayHandled(pid)) {
      HEPEnergyType E = vP.GetEnergy();
      HEPMassType m = vP.GetMass();

      const double gamma = E / m;

      const TimeType t0 = particles::GetLifetime(vP.GetPID());
      auto const lifetime = gamma * t0;

      const auto mkin =
          (E * E - vP.GetMomentum().squaredNorm()); // delta_mass(vP.GetMomentum(), E, m);
      cout << "Sibyll::Decay: code: " << vP.GetPID() << endl;
      cout << "Sibyll::Decay: MinStep: t0: " << t0 << endl;
      cout << "Sibyll::Decay: MinStep: energy: " << E / 1_GeV << " GeV" << endl;
      cout << "Sibyll::Decay: momentum: " << vP.GetMomentum().GetComponents() / 1_GeV
           << " GeV" << endl;
      cout << "Sibyll::Decay: momentum: shell mass-kin. inv. mass "
           << mkin / 1_GeV / 1_GeV << " " << m / 1_GeV * m / 1_GeV << endl;
      auto sib_id = process::sibyll::ConvertToSibyllRaw(vP.GetPID());
      cout << "Sibyll::Decay: sib mass: " << get_sibyll_mass2(sib_id) << endl;
      cout << "Sibyll::Decay: MinStep: gamma: " << gamma << endl;
      cout << "Sibyll::Decay: MinStep: tau (s): " << lifetime / 1_s << endl;

      return lifetime;
    } else
      return std::numeric_limits<double>::infinity() * 1_s;
  }

  template <>
  void Decay::DoDecay(SetupView& view) {
    using geometry::Point;
    using namespace units::si;

    auto const projectile = view.GetProjectile();

    const particles::Code pCode = projectile.GetPID();
    // check if sibyll is configured to handle this decay!
    if (!IsDecayHandled(pCode))
      throw std::runtime_error("STOP! Sibyll not configured to execute this decay!");

    fCount++;
    SibStack ss;
    ss.Clear();

    // copy particle to sibyll stack
    ss.AddParticle(process::sibyll::ConvertToSibyllRaw(pCode), projectile.GetEnergy(),
                   projectile.GetMomentum(),
                   // setting particle mass with Corsika values, may be inconsistent
                   // with sibyll internal values
                   particles::GetMass(pCode));
    // remember position
    Point const decayPoint = projectile.GetPosition();
    TimeType const t0 = projectile.GetTime();
    // remember if particles is unstable
    // auto const priorIsUnstable = IsUnstable(pCode);
    // switch on decay for this particle
    SetUnstable(pCode);
    PrintDecayConfig(pCode);

    // call sibyll decay
    cout << "Decay: calling Sibyll decay routine.." << endl;
    decsib_();

    // print output
    int print_unit = 6;
    sib_list_(print_unit);

    // reset to stable
    SetStable(pCode);

    // copy particles from sibyll stack to corsika
    for (auto& psib : ss) {
      // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
      if (psib.HasDecayed()) continue;
      // add to corsika stack
      view.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                geometry::Point, units::si::TimeType>{
              process::sibyll::ConvertFromSibyll(psib.GetPID()), psib.GetEnergy(),
              psib.GetMomentum(), decayPoint, t0});
    }
    // empty sibyll stack
    ss.Clear();
  }

} // namespace corsika::process::sibyll
