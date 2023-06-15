
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
#include <corsika/modules/proposal/InteractionModel.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/Logging.hpp>

namespace corsika::proposal {

  template <typename TOutput>
  inline void ContinuousProcess<TOutput>::buildCalculator(
      Code code, NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");
    if (code == Code::Photon) return; // no continuous builders needed for photons

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto const emCut = get_energy_production_threshold(
        code); //! energy resolutions globally defined for individual particles
    auto c = p_cross->second(media.at(comp.getHash()), emCut);

    // Use higland multiple scattering and deactivate stochastic deflection by
    // passing an empty vector
    static constexpr auto ms_type = PROPOSAL::MultipleScatteringType::MoliereInterpol;
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

  template <typename TOutput>
  template <typename TEnvironment, typename... TOutputArgs>
  inline ContinuousProcess<TOutput>::ContinuousProcess(TEnvironment const& _env,
                                                       TOutputArgs&&... args)
      : TOutput(args...)
      , ProposalProcessBase(_env) {}

  template <typename TOutput>
  template <typename TParticle>
  inline void ContinuousProcess<TOutput>::scatter(Step<TParticle>& step,
                                                  HEPEnergyType const& loss,
                                                  GrammageType const& grammage) {

    // get or build corresponding calculators
    auto c = getCalculator(step.getParticlePre(), calc);

    // Cast corsika vector to proposal vector
    auto particle_dir = step.getDirectionPre();
    auto d = particle_dir.getComponents();
    auto direction = PROPOSAL::Cartesian3D(d.getX().magnitude(), d.getY().magnitude(),
                                           d.getZ().magnitude());

    auto E_i_total = step.getEkinPre() + step.getParticlePre().getMass();
    auto E_f_total = E_i_total - loss;

    // draw random numbers required for scattering process
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = std::array<double, 4>();
    for (auto& it : rnd) it = distr(RNG_);

    // calculate deflection based on particle energy, loss
    auto deflection = (c->second).scatter->CalculateMultipleScattering(
        grammage / 1_g * square(1_cm), E_i_total / 1_MeV, E_f_total / 1_MeV, rnd);

    [[maybe_unused]] auto [unused1, final_direction] =
        PROPOSAL::multiple_scattering::ScatterInitialDirection(direction, deflection);

    // update particle direction after continuous loss caused by multiple
    // scattering
    DirectionVector dU_{
        particle_dir.getCoordinateSystem(),
        {final_direction.GetX(), final_direction.GetY(), final_direction.GetZ()}};
    DirectionVector diff_dir_ = dU_ - particle_dir;
    step.add_dU(diff_dir_);
  }

  template <typename TOutput>
  template <typename TParticle>
  inline ProcessReturn ContinuousProcess<TOutput>::doContinuous(Step<TParticle>& step,
                                                                bool const) {
    if (!canInteract(step.getParticlePre().getPID())) return ProcessReturn::Ok;
    if (step.getDisplacement().getSquaredNorm() == static_pow<2>(0_m))
      return ProcessReturn::Ok;
    if (step.getParticlePre().getPID() == Code::Photon)
      return ProcessReturn::Ok; // no continuous energy losses, no scattering for photons
    // calculate passed grammage
    auto dX = step.getParticlePre().getNode()->getModelProperties().getIntegratedGrammage(
        step.getStraightTrack());

    // get or build corresponding track integral calculator and solve the
    // integral
    auto c = getCalculator(step.getParticlePre(), calc);
    auto E_i_total = (step.getEkinPre() + step.getParticlePre().getMass());
    auto E_f_total = (c->second).disp->UpperLimitTrackIntegral(
                         E_i_total * (1 / 1_MeV), dX * ((1 / 1_g) * 1_cm * 1_cm)) *
                     1_MeV;
    auto dE = E_i_total - E_f_total;

    // if the particle has a charge take multiple scattering into account
    if (step.getParticlePre().getChargeNumber() != 0) scatter(step, dE, dX);
    step.add_dEkin(-dE); // on the stack, this is just kinetic energy, E-m

    // also send to output
    TOutput::write(step.getPositionPre(), step.getPositionPost(),
                   step.getParticlePre().getPID(),
                   step.getParticlePre().getWeight() * dE);

    return ProcessReturn::Ok;
  }

  template <typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline LengthType ContinuousProcess<TOutput>::getMaxStepLength(
      TParticle const& vP, TTrajectory const& track) {
    auto const code = vP.getPID();
    if (!canInteract(code)) return meter * std::numeric_limits<double>::infinity();
    if (code == Code::Photon)
      return meter *
             std::numeric_limits<double>::infinity(); // no step limitation for photons

    // Limit the step size of a conitnuous loss. The maximal continuous loss seems to be
    // a hyper parameter which must be adjusted.
    //
    auto const energy = vP.getEnergy();
    auto const energy_lim = std::max(
        energy * 0.9, // either 10% relative loss max., or
        (get_kinetic_energy_propagation_threshold(code) +
         get_mass(code)) // energy thresholds globally defined for individual particles
            * 0.9999     // need to go slightly below global e-cut to assure removal
                         // in ParticleCut. This does not matter since at cut-time
                         // the entire energy is removed.
    );

    // solving the track integral for giving energy lim
    auto c = getCalculator(vP, calc);
    auto grammage =
        (c->second).disp->SolveTrackIntegral(energy / 1_MeV, energy_lim / 1_MeV) * 1_g /
        square(1_cm);

    // return it in distance aequivalent
    auto dist =
        vP.getNode()->getModelProperties().getArclengthFromGrammage(track, grammage);
    CORSIKA_LOG_TRACE("PROPOSAL::getMaxStepLength X={} g/cm2, l={} m ",
                      grammage / 1_g * square(1_cm), dist / 1_m);
    if (dist < 0_m) {
      CORSIKA_LOG_WARN(
          "PROPOSAL::getMaxStepLength calculated a negative step length of l={} m. "
          "Return 0_m instead.",
          dist / 1_m);
      return 0_m;
    }
    return dist;
  }

  template <typename TOutput>
  inline YAML::Node ContinuousProcess<TOutput>::getConfig() const {
    return YAML::Node();
  }

} // namespace corsika::proposal
