/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/proposal/ContinuousProcess.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

namespace corsika::process::proposal {

  using namespace corsika::units::si;

  void ContinuousProcess::BuildCalculator(particles::Code code,
                                          environment::NuclearComposition const& comp) {
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");
    auto c = p_cross->second(media.at(&comp), cut);
    auto disp = PROPOSAL::make_displacement(c, true);
    auto scatter = PROPOSAL::make_scattering("highland", particle[code], media.at(&comp));
    calc[std::make_pair(&comp, code)] =
        std::make_tuple(std::move(disp), std::move(scatter));
  }

  template <>
  ContinuousProcess::ContinuousProcess(setup::SetupEnvironment const& _env,
                                       particle_cut::ParticleCut& _cut)
      : ProposalProcessBase(_env, _cut) {}

  template <>
  HEPEnergyType ContinuousProcess::TotalEnergyLoss(setup::Stack::ParticleType const& vP,
                                                   GrammageType const& vDX) {
    auto c = GetCalculator(vP, calc);
    return vP.GetEnergy() - get<DISPLACEMENT>(c->second)->UpperLimitTrackIntegral(
                                vP.GetEnergy() / 1_MeV, vDX / 1_g * 1_cm * 1_cm) *
                                1_MeV;
  }

  template <>
  void ContinuousProcess::Scatter(setup::Stack::ParticleType& vP,
                                  HEPEnergyType const& loss,
                                  GrammageType const& grammage) {
    auto c = GetCalculator(vP, calc);
    auto d = vP.GetDirection().GetComponents();
    auto direction = PROPOSAL::Vector3D(d.GetX().magnitude(), d.GetY().magnitude(),
                                        d.GetZ().magnitude());
    auto E_f = vP.GetEnergy() - loss; // final energy
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = array<double, 4>();
    for (auto& it : rnd) it = distr(fRNG);
    auto [mean_dir, final_dir] = get<SCATTERING>(c->second)->Scatter(
        grammage / 1_g * square(1_cm), vP.GetEnergy() / 1_MeV, E_f / 1_MeV, direction,
        rnd);
    (void)mean_dir;
    auto vec = corsika::geometry::QuantityVector(
        final_dir.GetX() * E_f, final_dir.GetY() * E_f, final_dir.GetZ() * E_f);
    vP.SetMomentum(corsika::stack::MomentumVector(
        corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem(),
        vec));
  }

  template <>
  EProcessReturn ContinuousProcess::DoContinuous(setup::Stack::ParticleType& vP,
                                                 setup::Trajectory const& vT) {
    if (!CanInteract(vP.GetPID())) return process::EProcessReturn::eOk;
    auto dX = vP.GetNode()->GetModelProperties().IntegratedGrammage(vT, vT.GetLength());
    auto energy_loss = TotalEnergyLoss(vP, dX);
    if (vP.GetChargeNumber() != 0) Scatter(vP, energy_loss, dX);
    vP.SetEnergy(vP.GetEnergy() - energy_loss);
    if (vP.GetEnergy() < cut.GetECut()) return process::EProcessReturn::eParticleAbsorbed;
    vP.SetMomentum(vP.GetMomentum() * vP.GetEnergy() / vP.GetMomentum().GetNorm());
    return process::EProcessReturn::eOk;
  }

  template <>
  units::si::LengthType ContinuousProcess::MaxStepLength(
      setup::Stack::ParticleType const& vP, setup::Trajectory const& vT) {
    if (!CanInteract(vP.GetPID()))
      return units::si::meter * std::numeric_limits<double>::infinity();
    auto energy_lim = 0.9 * vP.GetEnergy();
    if (cut.GetECut() > energy_lim) energy_lim = cut.GetECut();
    auto c = GetCalculator(vP, calc);
    auto grammage = get<DISPLACEMENT>(c->second)->SolveTrackIntegral(
                        vP.GetEnergy() / 1_MeV, energy_lim / 1_MeV) *
                    1_g / square(1_cm);
    return vP.GetNode()->GetModelProperties().ArclengthFromGrammage(vT, grammage) *
           1.0001;
  }

} // namespace corsika::process::proposal
