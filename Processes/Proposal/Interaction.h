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
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/proposal/ProposalProcessBase.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <array>

namespace corsika::process::proposal {

  using namespace corsika::units::si;

  class Interaction : public InteractionProcess<Interaction>, ProposalProcessBase {

    using calculator_t = tuple<unique_ptr<PROPOSAL::SecondariesCalculator>,
                               unique_ptr<PROPOSAL::Interaction>>;

    std::unordered_map<calc_key_t, calculator_t, hash> calc;

    void BuildCalculator(particles::Code, environment::NuclearComposition const&) final;

    enum { SECONDARIES, INTERACTION };

  public:
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, particle_cut::ParticleCut& cut);

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const& p);
  };
} // namespace corsika::process::proposal
