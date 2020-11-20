/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Decay.hpp>
#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/modules/sibyll/SibStack.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iostream>
#include <vector>

using SetupView = corsika::setup::StackView;
using SetupProjectile = corsika::setup::StackView::ParticleType;
using SetupParticle = corsika::setup::Stack::ParticleType;

namespace corsika::sibyll {

  Decay::Decay() {}
  Decay::~Decay() { std::cout << "Sibyll::Decay n=" << fCount << std::endl; }
  void Decay::Init() {
    // switch off decays to avoid internal decay chains
    SetAllStable();
  }

  void Decay::SetStable(const std::vector<corsika::Code> vParticleList) {
    for (auto p : vParticleList) Decay::SetStable(p);
  }

  void Decay::SetUnstable(const std::vector<corsika::Code> vParticleList) {
    for (auto p : vParticleList) Decay::SetUnstable(p);
  }

  bool Decay::IsStable(const corsika::Code vCode) {
    return abs(corsika::sibyll::ConvertToSibyllRaw(vCode)) <= 0 ? true : false;
  }

  bool Decay::IsUnstable(const corsika::Code vCode) {
    return abs(corsika::sibyll::ConvertToSibyllRaw(vCode)) > 0 ? true : false;
  }

  void Decay::SetDecay(const corsika::Code vCode, const bool vMakeUnstable) {
    vMakeUnstable ? SetUnstable(vCode) : SetStable(vCode);
  }

  void Decay::SetUnstable(const corsika::Code vCode) {
    std::cout << "Sibyll::Interaction: setting " << vCode << " unstable.." << std::endl;
    const int s_id = abs(corsika::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetStable(const corsika::Code vCode) {
    std::cout << "Sibyll::Interaction: setting " << vCode << " stable.." << std::endl;
    const int s_id = abs(corsika::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Decay::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
  }

  void Decay::SetAllUnstable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = abs(s_csydec_.idb[i]);
  }

  void Decay::PrintDecayConfig(const corsika::Code vCode) {
    std::cout << "Decay: Sibyll decay configuration:" << std::endl;
    const int sibCode = corsika::sibyll::ConvertToSibyllRaw(vCode);
    const int absSibCode = abs(sibCode);
    std::cout << vCode << " is ";
    if (s_csydec_.idb[absSibCode - 1] <= 0)
      std::cout << "stable" << std::endl;
    else
      std::cout << "unstable" << std::endl;
  }

  template <>
  TimeType Decay::GetLifetime(SetupParticle const& vP) const {

    HEPEnergyType E = vP.GetEnergy();
    HEPMassType m = vP.GetMass();

    const double gamma = E / m;

    const TimeType t0 = corsika::lifetime(vP.GetPID());
    auto const lifetime = gamma * t0;

    const auto mkin =
        (E * E - vP.GetMomentum().squaredNorm()); // delta_mass(vP.GetMomentum(), E, m);
    std::cout << "Decay: code: " << vP.GetPID() << std::endl;
    std::cout << "Decay: MinStep: t0: " << t0 << std::endl;
    std::cout << "Decay: MinStep: energy: " << E / 1_GeV << " GeV" << std::endl;
    std::cout << "Decay: momentum: " << vP.GetMomentum().GetComponents() / 1_GeV << " GeV"
              << std::endl;
    std::cout << "Decay: momentum: shell mass-kin. inv. mass " << mkin / 1_GeV / 1_GeV
              << " " << m / 1_GeV * m / 1_GeV << std::endl;
    auto sib_id = corsika::sibyll::ConvertToSibyllRaw(vP.GetPID());
    std::cout << "Decay: sib mass: " << get_sibyll_mass2(sib_id) << std::endl;
    std::cout << "Decay: MinStep: gamma: " << gamma << std::endl;
    std::cout << "Decay: MinStep: tau: " << lifetime << std::endl;

    return lifetime;
  }

  template <>
  void Decay::DoDecay(SetupProjectile& vP) {
    using corsika::Point;

    fCount++;
    SibStack ss;
    ss.Clear();
    const corsika::Code pCode = vP.GetPID();
    // copy particle to sibyll stack
    ss.AddParticle(corsika::sibyll::ConvertToSibyllRaw(pCode), vP.GetEnergy(),
                   vP.GetMomentum(),
                   // setting particle mass with Corsika values, may be inconsistent
                   // with sibyll internal values
                   corsika::get_mass(pCode));
    // remember position
    Point const decayPoint = vP.GetPosition();
    TimeType const t0 = vP.GetTime();
    // remember if particles is unstable
    // auto const priorIsUnstable = IsUnstable(pCode);
    // switch on decay for this particle
    SetUnstable(pCode);
    PrintDecayConfig(pCode);

    // call sibyll decay
    std::cout << "Decay: calling Sibyll decay routine.." << std::endl;
    decsib_();

    // reset to stable
    SetStable(pCode);
    // print output
    int print_unit = 6;
    sib_list_(print_unit);

    // copy particles from sibyll stack to corsika
    for (const auto& psib : ss) {
      // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
      if (psib.HasDecayed()) continue;
      // add to corsika stack
      vP.AddSecondary(
          std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, Point,
                     TimeType>{corsika::sibyll::ConvertFromSibyll(psib.GetPID()),
                               psib.GetEnergy(), psib.GetMomentum(), decayPoint, t0});
    }
    // empty sibyll stack
    ss.Clear();
  }

} // namespace corsika::sibyll
