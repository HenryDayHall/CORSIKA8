
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

namespace corsika::proposal {

  inline void ContinuousProcess::buildCalculator(Code code,
                                                 NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto const emCut =
        get_kinetic_energy_threshold(code) +
        get_mass(code); //! energy thresholds globally defined for individual particles
    auto c = p_cross->second(media.at(comp.getHash()), emCut);

    // Use higland multiple scattering and deactivate stochastic deflection by
    // passing an empty vector
    static constexpr auto ms_type = PROPOSAL::MultipleScatteringType::Highland;
    auto s_type = std::vector<PROPOSAL::InteractionType>();

    // Build displacement integral and scattering object and interpolate them too and
    // saved in the calc map by a key build out of a hash of composed of the component and
    // particle code.
    auto calculator =
        Calculator{PROPOSAL::make_displacement(c, true),
                   PROPOSAL::make_scattering(ms_type, s_type, particle[code],
                                             media.at(comp.getHash()))};
    calc[std::make_pair(comp.getHash(), code)] = std::move(calculator);
  }

  template <typename TEnvironment>
  inline ContinuousProcess::ContinuousProcess(TEnvironment const& _env)
      : ProposalProcessBase(_env) {}

  template <typename TParticle>
  inline void ContinuousProcess::scatter(TParticle& vP, HEPEnergyType const& loss,
                                         GrammageType const& grammage) {

    // get or build corresponding calculators
    auto c = getCalculator(vP, calc);

    // Cast corsika vector to proposal vector
    auto vP_dir = vP.getDirection();
    auto d = vP_dir.getComponents();
    auto direction = PROPOSAL::Cartesian3D(d.getX().magnitude(), d.getY().magnitude(),
                                           d.getZ().magnitude());

    auto E_f = vP.getEnergy() - loss;

    // draw random numbers required for scattering process
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = std::array<double, 4>();
    for (auto& it : rnd) it = distr(RNG_);

    // calculate deflection based on particle energy, loss
    auto deflection = (c->second).scatter->CalculateMultipleScattering(
        grammage / 1_g * square(1_cm), vP.getEnergy() / 1_MeV, E_f / 1_MeV, rnd);

    [[maybe_unused]] auto [unused1, final_direction] =
        PROPOSAL::multiple_scattering::ScatterInitialDirection(direction, deflection);

    // update particle direction after continuous loss caused by multiple
    // scattering
    vP.setDirection(
        {vP_dir.getCoordinateSystem(),
         {final_direction.GetX(), final_direction.GetY(), final_direction.GetZ()}});
  }

  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ContinuousProcess::doContinuous(TParticle& vP,
                                                       TTrajectory const& vT,
                                                       bool const) {
    if (!canInteract(vP.getPID())) return ProcessReturn::Ok;
    if (vT.getLength() == 0_m) return ProcessReturn::Ok;

    // calculate passed grammage
    auto dX = vP.getNode()->getModelProperties().getIntegratedGrammage(vT);

    // get or build corresponding track integral calculator and solve the
    // integral
    auto c = getCalculator(vP, calc);
    auto final_energy = (c->second).disp->UpperLimitTrackIntegral(
                            vP.getEnergy() / 1_MeV, dX / 1_g * 1_cm * 1_cm) *
                        1_MeV;
    auto dE = vP.getEnergy() - final_energy;
    energy_lost_ += dE;

    // if the particle has a charge take multiple scattering into account
    if (vP.getChargeNumber() != 0) scatter(vP, dE, dX);
    vP.setEnergy(final_energy); // on the stack, this is just kinetic energy, E-m
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrajectory>
  inline LengthType ContinuousProcess::getMaxStepLength(TParticle const& vP,
                                                        TTrajectory const& vT) {
    auto const code = vP.getPID();
    if (!canInteract(code)) return meter * std::numeric_limits<double>::infinity();

    // Limit the step size of a conitnuous loss. The maximal continuous loss seems to be
    // a hyper parameter which must be adjusted.
    //
    auto const energy = vP.getEnergy();
    auto const energy_lim =
        std::max(energy * 0.9, // either 10% relative loss max., or
                 get_kinetic_energy_threshold(
                     code) // energy thresholds globally defined for individual particles
                     * 0.9999 // need to go slightly below global e-cut to assure removal
                              // in ParticleCut. This does not matter since at cut-time
                              // the entire energy is removed.
        );

    // solving the track integral for giving energy lim
    auto c = getCalculator(vP, calc);
    auto grammage =
        (c->second).disp->SolveTrackIntegral(energy / 1_MeV, energy_lim / 1_MeV) * 1_g /
        square(1_cm);

    // return it in distance aequivalent
    auto dist = vP.getNode()->getModelProperties().getArclengthFromGrammage(vT, grammage);
    CORSIKA_LOG_TRACE("PROPOSAL::getMaxStepLength X={} g/cm2, l={} m ",
                      grammage / 1_g * square(1_cm), dist / 1_m);
    return dist;
  }

  inline void ContinuousProcess::showResults() const {
    CORSIKA_LOG_DEBUG(
        " ******************************\n"
        " PROCESS::ContinuousProcess: \n"
        " energy lost dE (GeV)      :  {}",
        energy_lost_ / 1_GeV);
  }

  inline void ContinuousProcess::reset() { energy_lost_ = 0_GeV; }

} // namespace corsika::proposal
