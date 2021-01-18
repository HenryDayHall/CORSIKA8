/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/media/Environment.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>

#include <corsika/modules/proposal/ProposalProcessBase.hpp>

#include <unordered_map>

namespace corsika::proposal {

  //!
  //! Electro-magnetic and gamma continous losses produced by proposal. It makes
  //! use of interpolation tables which are runtime intensive calculation, but can be
  //! reused by setting the \param PROPOSAL::InterpolationDef::path_to_tables variable.
  //!
  class ContinuousProcess
      : public corsika::ContinuousProcess<proposal::ContinuousProcess>,
        ProposalProcessBase {

    enum { eDISPLACEMENT, eSCATTERING };
    using calc_t = std::tuple<std::unique_ptr<PROPOSAL::Displacement>,
                              std::unique_ptr<PROPOSAL::Scattering>>;

    std::unordered_map<calc_key_t, calc_t, hash>
        calc; //!< Stores the displacement and scattering calculators.

    HEPEnergyType energy_lost_ = 0 * electronvolt;

    //!
    //! Build the displacement and scattering calculators and add it to calc.
    //!
    void buildCalculator(Code, NuclearComposition const&) final;

  public:
    //!
    //! Produces the continuous loss calculator for leptons based on nuclear
    //! compositions and stochastic description limited by the particle cut.
    //!
    template <typename TEnvironment>
    ContinuousProcess(TEnvironment const&);

    //!
    //! Multiple Scattering of the lepton. Stochastic deflection is not yet taken into
    //! account. Displacment of the track due to multiple scattering is not possible
    //! because of the constant referernce. The final direction will be updated anyway.
    //!
    template <typename TParticle>
    void scatter(TParticle&, HEPEnergyType const&, GrammageType const&);

    //!
    //! Produces the loss and deflection after given distance for the particle.
    //! If the particle if below the given energy threshold where it will be
    //! considered stochastically, it will be absorbed.
    //!
    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(TParticle&, TTrack const&);

    //!
    //! Calculates maximal step length of process.
    //!
    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const&, TTrack const&);

    void showResults() const;
    void reset();
    HEPEnergyType getEnergyLost() const { return energy_lost_; }
  };
} // namespace corsika::proposal

#include <corsika/detail/modules/proposal/ContinuousProcess.inl>
