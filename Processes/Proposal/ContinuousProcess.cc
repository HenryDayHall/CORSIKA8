/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/process/proposal/ContinuousProcess.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

#include <iostream>

namespace corsika::process::proposal {

  void ContinuousProcess::BuildCalculator(particles::Code code,
                                          environment::NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto c = p_cross->second(media.at(&comp), emCut_);

    // Build displacement integral and scattering object and interpolate them too and
    // saved in the calc map by a key build out of a hash of composed of the component and
    // particle code.
    auto disp = PROPOSAL::make_displacement(c, true);
    auto scatter = PROPOSAL::make_scattering("highland", particle[code], media.at(&comp));
    calc[std::make_pair(&comp, code)] =
        std::make_tuple(std::move(disp), std::move(scatter));
  }

  template <>
  ContinuousProcess::ContinuousProcess(setup::SetupEnvironment const& _env,
                                       corsika::units::si::HEPEnergyType _emCut)
      : ProposalProcessBase(_env, _emCut) {}

  template <>
  void ContinuousProcess::Scatter(setup::Stack::ParticleType& vP,
                                  corsika::units::si::HEPEnergyType const& loss,
                                  corsika::units::si::GrammageType const& grammage) {
    using namespace corsika::units::si; // required for operator::_MeV

    // Get or build corresponding calculators
    auto c = GetCalculator(vP, calc);

    // Cast corsika vector to proposal vector
    auto d = vP.GetDirection().GetComponents();
    auto direction = PROPOSAL::Vector3D(d.GetX().magnitude(), d.GetY().magnitude(),
                                        d.GetZ().magnitude());

    auto E_f = vP.GetEnergy() - loss;

    // draw random numbers required for scattering process
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = array<double, 4>();
    for (auto& it : rnd) it = distr(fRNG);

    // calculate deflection based on particle energy, loss
    auto [mean_dir, final_dir] = get<SCATTERING>(c->second)->Scatter(
        grammage / 1_g * square(1_cm), vP.GetEnergy() / 1_MeV, E_f / 1_MeV, direction,
        rnd);

    // TODO: neglect mean direction deflection because Trajectory is a const ref
    (void)mean_dir;

    // update particle direction after continuous loss caused by multiple
    // scattering
    auto vec = corsika::geometry::QuantityVector(
        final_dir.GetX() * E_f, final_dir.GetY() * E_f, final_dir.GetZ() * E_f);
    vP.SetMomentum(corsika::stack::MomentumVector(
        corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem(),
        vec));
  }

  template <>
  EProcessReturn ContinuousProcess::DoContinuous(setup::Stack::ParticleType& vP,
                                                 setup::Trajectory const& vT) {
    using namespace corsika::units::si; // required for operator::_MeV

    if (!CanInteract(vP.GetPID())) return process::EProcessReturn::eOk;
    if (vT.GetLength()==0_m) return process::EProcessReturn::eOk;

    // calculate passed grammage
    auto dX = vP.GetNode()->GetModelProperties().IntegratedGrammage(vT, vT.GetLength());

    // Get or build corresponding track integral calculator and solve the
    // integral
    auto c = GetCalculator(vP, calc);
    auto final_energy = get<DISPLACEMENT>(c->second)->UpperLimitTrackIntegral(
                            vP.GetEnergy() / 1_MeV, dX / 1_g * 1_cm * 1_cm) *
                        1_MeV;
    auto dE = vP.GetEnergy() - final_energy;
    energy_lost_ += dE;

    // if the particle has a charge take multiple scattering into account
    if (vP.GetChargeNumber() != 0) Scatter(vP, dE, dX);

    // Update the energy and absorbe the particle if it's below the energy
    // threshold, because it will no longer propagated.
    if (final_energy <= emCut_) {
      vP.SetEnergy(emCut_);
      vP.SetMomentum(vP.GetMomentum() * vP.GetEnergy() / vP.GetMomentum().GetNorm());
      return process::EProcessReturn::eParticleAbsorbed;
    }

    if (final_energy <= emCut_) {
      vP.SetEnergy(emCut_);
      vP.SetMomentum(vP.GetMomentum() * vP.GetEnergy() / vP.GetMomentum().GetNorm());
      return process::EProcessReturn::eParticleAbsorbed;
    }
    vP.SetEnergy(final_energy);
    vP.SetMomentum(vP.GetMomentum() * vP.GetEnergy() / vP.GetMomentum().GetNorm());
    return process::EProcessReturn::eOk;
  }

  template <>
  units::si::LengthType ContinuousProcess::MaxStepLength(
      setup::Stack::ParticleType const& vP, setup::Trajectory const& vT) {
    using namespace corsika::units::si; // required for operator::_MeV

    if (!CanInteract(vP.GetPID()))
      return units::si::meter * std::numeric_limits<double>::infinity();

    // Limit the step size of a conitnous loss. The maximal continuous loss seems to be a
    // hyper parameter which must be adjusted.
    //
    // in any case: never go below 0.99*emCut_ This needs to be
    // slightly smaller than emCut_ since, either this Step is limited
    // by energy_lim, then the particle is stopped in a very short
    // range (before doing anythin else) and is then removed
    // instantly. The exact position where it reaches emCut is not
    // important, the important fact is that its E_kin is zero
    // afterwards.
    //
    auto energy_lim = std::max(0.9 * vP.GetEnergy(), 0.99*emCut_);

    // solving the track integral for giving energy lim
    auto c = GetCalculator(vP, calc);
    auto grammage = get<DISPLACEMENT>(c->second)->SolveTrackIntegral(
                        vP.GetEnergy() / 1_MeV, energy_lim / 1_MeV) *
                    1_g / square(1_cm);

    // return it in distance aequivalent
    return vP.GetNode()->GetModelProperties().ArclengthFromGrammage(vT, grammage);
  }

  void ContinuousProcess::ShowResults() const {
    using namespace corsika::units::si; // required for operator::_MeV
    std::cout << " ******************************" << std::endl
              << " PROCESS::ContinuousProcess: " << std::endl;
    std::cout << " energy lost dE (GeV)      :  " << energy_lost_ / 1_GeV << std::endl;
  }

  void ContinuousProcess::Reset() {
    using namespace corsika::units::si; // required for operator::_MeV
    energy_lost_ = 0_GeV;
  }

} // namespace corsika::process::proposal
