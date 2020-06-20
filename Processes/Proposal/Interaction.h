/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_proposal_interaction_h_
#define _corsika_process_proposalythia_interaction_h_

#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <random>
#include <unordered_map>
#include "PROPOSAL/PROPOSAL.h"

using namespace corsika::environment;

using CORSIKA_ParticleCut = corsika::process::particle_cut::ParticleCut;

namespace corsika::process::proposal {

  /* static std::unordered_map<particles::Code, PROPOSAL::ParticleDef> particle_map{ */
  /*     {particles::Code::Gamma, PROPOSAL::GammaDef()}, */
  /*     {particles::Code::Electron, PROPOSAL::EMinusDef()}, */
  /*     {particles::Code::Positron, PROPOSAL::EPlusDef()}, */
  /*     {particles::Code::MuMinus, PROPOSAL::MuMinusDef()}, */
  /*     {particles::Code::MuPlus, PROPOSAL::MuPlusDef()}, */
  /*     {particles::Code::TauPlus, PROPOSAL::TauPlusDef()}, */
  /*     {particles::Code::TauMinus, PROPOSAL::TauMinusDef()}, */
  /* }; */

  template <class TEnvironment>
  class Interaction
      : public corsika::process::InteractionProcess<Interaction<TEnvironment>> {

  private:
    TEnvironment const& fEnvironment;
    shared_ptr<const PROPOSAL::EnergyCutSettings> cut;

    static std::unordered_map<particles::Code, PROPOSAL::ParticleDef> particle_map;
    std::unordered_map<const NuclearComposition*, PROPOSAL::Medium> medium_map;

    enum { SECONDARIES, INTERACTION, DISPLACEMENT };
    /* std::uniform_real_distribution<> rnd_uniform(0., 1.); */

    /* std::map<particles::Code, std::unique_ptr<PROPOSAL::UtilityInterpolantInteraction>>
     * corsika_particle_to_utility_map; */
    /* std::map<particles::Code,
     * std::unique_ptr<PROPOSAL::UtilityInterpolantDisplacement>>
     * corsika_particle_to_displacement_map; */

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

    // fTrackedParticles[proposal_particle.GetName()] return  particle::Code

    auto IsTracked(particles::Code pcode) const noexcept {
      auto search = particle_map.find(pcode);
      if (search != particle_map.end()) return true;
      return false;
    };

    using calculator_t =
        tuple<PROPOSAL::SecondariesCalculator, unique_ptr<PROPOSAL::Interaction>,
              unique_ptr<PROPOSAL::Displacement>>;
    std::unordered_map<const NuclearComposition*, calculator_t> calculators;

    template <typename Particle>
    auto GetCalculator(Particle&);

  public:
    Interaction(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle& p);
  };

} // namespace corsika::process::proposal
#endif
