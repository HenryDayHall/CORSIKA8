/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/proposal/ProposalProcessBase.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <array>

namespace corsika::process::proposal {

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
    void BuildCalculator(particles::Code, environment::NuclearComposition const&) final;

  public:
    //!
    //! Produces the stoachastic loss calculator for leptons based on nuclear
    //! compositions and stochastic description limited by the particle cut.
    //!
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, corsika::units::si::HEPEnergyType emCut);

    //!
    //! Calculate the rates for the different targets and interactions. Sample a
    //! pair of interaction-type, component and rate, followed by sampling a loss and
    //! produce the corresponding secondaries and store them on the particle stack.
    //!
    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    //!
    //! Calculates the  mean free path length
    //!
    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const& p);
  };
} // namespace corsika::process::proposal
