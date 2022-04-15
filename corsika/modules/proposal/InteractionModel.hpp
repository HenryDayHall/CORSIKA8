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
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/modules/proposal/ProposalProcessBase.hpp>
#include <corsika/modules/proposal/HadronicPhotonModel.hpp>

namespace corsika::proposal {

  //!
  //! Electro-magnetic and photon stochastic losses produced by proposal. It makes
  //! use of interpolation tables which are runtime intensive calculation, but can be
  //! reused by setting the \param PROPOSAL::InterpolationDef::path_to_tables variable.
  //! Hadroninc interactions of photons with nuclei are included. The cross section is
  //! calculated by PROPOSAL. For the production of hadronic secondaries an external model
  //! is needed that implements the doInteraction(TSecondaries& view, Code const
  //! projectile, Code const target,FourMomentum const& projectileP4, FourMomentum const&
  //! targetP4) routine.
  //! @tparam THadronicModel
  //!

  template <class THadronicModel>
  class InteractionModel : public ProposalProcessBase,
                           public HadronicPhotonModel<THadronicModel> {

    enum { eSECONDARIES, eINTERACTION };
    using calculator_t = std::tuple<std::unique_ptr<PROPOSAL::SecondariesCalculator>,
                                    std::unique_ptr<PROPOSAL::Interaction>>;

    std::unordered_map<calc_key_t, calculator_t, hash>
        calc_; //!< Stores the secondaries and interaction calculators.

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
    InteractionModel(TEnvironment const& env, THadronicModel&, HEPEnergyType const&);

    //!
    //! Calculate the rates for the different targets and interactions. Sample a
    //! pair of interaction-type, component and rate, followed by sampling a loss and
    //! produce the corresponding secondaries and store them on the particle stack.
    //!
    template <typename TSecondaryView>
    ProcessReturn doInteraction(TSecondaryView&, Code const projectileId,
                                FourMomentum const& projectileP4);

    //!
    //! Calculates and returns the cross section.
    //!
    template <typename TParticle>
    CrossSectionType getCrossSection(TParticle const& p, Code const projectileId,
                                     FourMomentum const& projectileP4);
  };

} // namespace corsika::proposal

#include <corsika/detail/modules/proposal/InteractionModel.inl>
