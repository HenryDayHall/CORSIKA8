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

#include "../../corsika/setup/SetupStack.hpp"
#include "../../corsika/setup/SetupTrajectory.hpp"

using std::make_tuple;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika;

using SetupView = corsika::StackView;
using SetupProjectile = corsika::StackView::ParticleType;
using SetupParticle = corsika::Stack::ParticleType;

namespace corsika::sibyll {

  Decay::Decay(const bool sibyll_printout_on)
      : sibyll_listing_(sibyll_printout_on) {
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

  Decay::~Decay() { C8LOG_DEBUG("Sibyll::Decay n={}", count_); }

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
    C8LOG_DEBUG("Sibyll::Decay: set to handle decay of {}", vParticleCode);
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
    C8LOG_DEBUG("Sibyll::Decay: setting {} unstable. ", vCode);
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetStable(const particles::Code vCode) {
    C8LOG_DEBUG("Sibyll::Decay: setting {} stable. ", vCode);
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
  }

  void Decay::SetAllUnstable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = abs(s_csydec_.idb[i]);
  }

  void Decay::PrintDecayConfig([[maybe_unused]] const particles::Code vCode) {
    [[maybe_unused]] const int sibCode = process::sibyll::ConvertToSibyllRaw(vCode);
    [[maybe_unused]] const int absSibCode = abs(sibCode);
    C8LOG_DEBUG("Decay: Sibyll decay configuration: {} is {}", vCode,
                (s_csydec_.idb[absSibCode - 1] <= 0) ? "stable" : "unstable");
  }

  void Decay::PrintDecayConfig() {
    C8LOG_DEBUG("Sibyll::Decay: decay configuration:");
    if (handleAllDecays_) {
      C8LOG_DEBUG("     all particles known to Sibyll are handled by Sibyll::Decay!");
    } else {
      for ([[maybe_unused]] auto& pCode : handledDecays_) {
        C8LOG_DEBUG("      Decay of {}  is handled by Sibyll!", pCode);
      }
    }
  }

  template <>
  units::si::TimeType Decay::GetLifetime(Particle const& vP) {
    using namespace units::si;

    const particles::Code pid = vP.GetPID();
    if (Decay::IsDecayHandled(pid)) {
      HEPEnergyType E = vP.GetEnergy();
      HEPMassType m = vP.GetMass();

      const double gamma = E / m;

      const TimeType t0 = particles::GetLifetime(vP.GetPID());
      auto const lifetime = gamma * t0;

      [[maybe_unused]] const auto mkin =
          (E * E - vP.GetMomentum().squaredNorm()); // delta_mass(vP.GetMomentum(), E, m);
      C8LOG_DEBUG("Sibyll::Decay: code: {} ", vP.GetPID());
      C8LOG_DEBUG("Sibyll::Decay: MinStep: t0: {} ", t0);
      C8LOG_DEBUG("Sibyll::Decay: MinStep: energy: {} GeV ", E / 1_GeV);
      C8LOG_DEBUG("Sibyll::Decay: momentum: {} GeV ",
                  vP.GetMomentum().GetComponents() / 1_GeV);
      C8LOG_DEBUG("Sibyll::Decay: momentum: shell mass-kin. inv. mass {} {}",
                  mkin / 1_GeV / 1_GeV, m / 1_GeV * m / 1_GeV);
      [[maybe_unused]] auto sib_id = process::sibyll::ConvertToSibyllRaw(vP.GetPID());
      C8LOG_DEBUG("Sibyll::Decay: sib mass: {}", get_sibyll_mass2(sib_id));
      C8LOG_DEBUG("Sibyll::Decay: MinStep: gamma:  {}", gamma);
      C8LOG_DEBUG("Sibyll::Decay: MinStep: tau {} s: ", lifetime / 1_s);

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

    count_++;
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
    C8LOG_DEBUG("Decay: calling Sibyll decay routine..");
    decsib_();

    if (sibyll_listing_) {
      // print output
      int print_unit = 6;
      sib_list_(print_unit);
    }

    // reset to stable
    SetStable(pCode);

    // copy particles from sibyll stack to corsika
    for (auto& psib : ss) {
      // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
      if (psib.HasDecayed()) continue;
      // add to corsika stack
      vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                geometry::Point, units::si::TimeType>{
              process::sibyll::ConvertFromSibyll(psib.GetPID()), psib.GetEnergy(),
              psib.GetMomentum(), decayPoint, t0});
    }
    // empty sibyll stack
    ss.Clear();
  }

} // namespace corsika::sibyll
