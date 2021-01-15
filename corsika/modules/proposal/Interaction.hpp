/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>

#include <corsika/media/Environment.hpp>

#include <corsika/modules/proposal/ProposalProcessBase.hpp>

#include <array>

namespace corsika::proposal {

  //!
  //! Electro-magnetic and gamma stochastic losses produced by proposal. It makes
  //! use of interpolation tables which are runtime intensive calculation, but can be
  //! reused by setting the \param PROPOSAL::InterpolationDef::path_to_tables variable.
  //!
  class Interaction : public InteractionProcess<Interaction>, ProposalProcessBase {

    enum { eSECONDARIES, eINTERACTION };
    using calculator_t = tuple<unique_ptr<PROPOSAL::SecondariesCalculator>,
                               unique_ptr<PROPOSAL::Interaction>>;

    std::unordered_map<calc_key_t, calculator_t, hash>
        calc; //!< Stores the secondaries and interaction calculators.

    //!
    //! Build the secondaries and interaction calculators and add it to calc.
    //!
    void buildCalculator(Code, NuclearComposition const&) final;

  public:
    //!
    //! Produces the stoachastic loss calculator for leptons based on nuclear
    //! compositions and stochastic description limited by the particle cut.
    //!
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, HEPEnergyType emCut);

    //!
    //! Calculate the rates for the different targets and interactions. Sample a
    //! pair of interaction-type, component and rate, followed by sampling a loss and
    //! produce the corresponding secondaries and store them on the particle stack.
    //!
    template <typename TSecondaryView>
    ProcessReturn doInteraction(TSecondaryView&);

    //!
    //! Calculates the  mean free path length
    //!
    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const& p);
  };
} // namespace corsika::proposal

#include <corsika/detail/modules/proposal/Interaction.inl>
