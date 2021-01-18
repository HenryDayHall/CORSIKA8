/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/media/IMediumModel.hpp>
#include <corsika/modules/proposal/ContinuousProcess.hpp>
#include <corsika/modules/proposal/Interaction.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iostream>

namespace corsika::proposal {

  void ContinuousProcess::buildCalculator(Code code, NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");
    
    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto const emCut = get_energy_threshold(
        code); //! energy thresholds globally defined for individual particles
    auto c = p_cross->second(media.at(comp.getHash()), emCut);

    // Build displacement integral and scattering object and interpolate them too and
    // saved in the calc map by a key build out of a hash of composed of the component and
    // particle code.
    auto disp = PROPOSAL::make_displacement(c, true);
    auto scatter =
        PROPOSAL::make_scattering("highland", particle[code], media.at(comp.getHash()));
    calc[std::make_pair(comp.getHash(), code)] =
        std::make_tuple(std::move(disp), std::move(scatter));
  }

  template <>
  ContinuousProcess::ContinuousProcess(setup::Environment const& _env
                                      )
      : ProposalProcessBase(_env) {}

  template <>
  void ContinuousProcess::scatter(setup::Stack::particle_type& vP,
                                  HEPEnergyType const& loss,
                                  GrammageType const& grammage) {

    // get or build corresponding calculators
    auto c = getCalculator(vP, calc);

    // Cast corsika vector to proposal vector
    auto vP_dir = vP.getDirection();
    auto d = vP_dir.getComponents();
    auto direction = PROPOSAL::Vector3D(d.getX().magnitude(), d.getY().magnitude(),
                                        d.getZ().magnitude());

    auto E_f = vP.getEnergy() - loss;

    // draw random numbers required for scattering process
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = array<double, 4>();
    for (auto& it : rnd) it = distr(RNG_);

    // calculate deflection based on particle energy, loss
    auto [mean_dir, final_dir] = get<eSCATTERING>(c->second)->Scatter(
        grammage / 1_g * square(1_cm), vP.getEnergy() / 1_MeV, E_f / 1_MeV, direction,
        rnd);

    // TODO: neglect mean direction deflection because Trajectory is a const ref
    (void)mean_dir;

    // update particle direction after continuous loss caused by multiple
    // scattering
    auto vec = QuantityVector(final_dir.GetX() * E_f, final_dir.GetY() * E_f,
                              final_dir.GetZ() * E_f);
    vP.setMomentum(MomentumVector(vP_dir.getCoordinateSystem(), vec));
  }

  template <>
  ProcessReturn ContinuousProcess::doContinuous(setup::Stack::particle_type& vP,
                                                setup::Trajectory const& vT) {

    if (!canInteract(vP.getPID())) return ProcessReturn::Ok;
    if (vT.getLength() == 0_m) return ProcessReturn::Ok;

    // calculate passed grammage
    auto dX =
        vP.getNode()->getModelProperties().getIntegratedGrammage(vT, vT.getLength());

    // get or build corresponding track integral calculator and solve the
    // integral
    auto c = getCalculator(vP, calc);
    auto final_energy = get<eDISPLACEMENT>(c->second)->UpperLimitTrackIntegral(
                            vP.getEnergy() / 1_MeV, dX / 1_g * 1_cm * 1_cm) *
                        1_MeV;
    auto dE = vP.getEnergy() - final_energy;
    energy_lost_ += dE;

    // if the particle has a charge take multiple scattering into account
    if (vP.getChargeNumber() != 0) scatter(vP, dE, dX);
    vP.setEnergy(final_energy);
    vP.setMomentum(vP.getMomentum() * vP.getEnergy() / vP.getMomentum().getNorm());
    return ProcessReturn::Ok;
  }

  template <>
  LengthType ContinuousProcess::getMaxStepLength(setup::Stack::particle_type const& vP,
                                                 setup::Trajectory const& vT) {
    auto const code = vP.getPID();
    if (!canInteract(code)) return meter * std::numeric_limits<double>::infinity();

    // Limit the step size of a conitnuous loss. The maximal continuous loss seems to be a
    // hyper parameter which must be adjusted.
    //
    auto const emCut = get_energy_threshold(
        code); //! energy thresholds globally defined for individual particles

    // in any case: never go below 0.99*emCut This needs to be
    // slightly smaller than emCut since, either this Step is limited
    // by energy_lim, then the particle is stopped in a very short
    // range (before doing anythin else) and is then removed
    // instantly. The exact position where it reaches emCut is not
    // important, the important fact is that its E_kin is zero
    // afterwards.
    //
    auto energy_lim = std::max(0.9 * vP.getEnergy(), 0.99 * emCut);

    // solving the track integral for giving energy lim
    auto c = getCalculator(vP, calc);
    auto grammage = get<eDISPLACEMENT>(c->second)->SolveTrackIntegral(
                        vP.getEnergy() / 1_MeV, energy_lim / 1_MeV) *
                    1_g / square(1_cm);

    // return it in distance aequivalent
    auto dist = vP.getNode()->getModelProperties().getArclengthFromGrammage(vT, grammage);
    CORSIKA_LOG_TRACE("PROPOSAL::getMaxStepLength X={} g/cm2, l={} m ",
                      grammage / 1_g * square(1_cm), dist / 1_m);
    return dist;
  }

  void ContinuousProcess::showResults() const {
    std::cout << " ******************************" << std::endl
              << " PROCESS::ContinuousProcess: " << std::endl;
    std::cout << " energy lost dE (GeV)      :  " << energy_lost_ / 1_GeV << std::endl;
  }

  void ContinuousProcess::reset() { energy_lost_ = 0_GeV; }

} // namespace corsika::proposal
