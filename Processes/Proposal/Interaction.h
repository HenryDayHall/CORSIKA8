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

  template <class TEnvironment>
  class Interaction
      : public corsika::process::InteractionProcess<Interaction<TEnvironment>> {

  private:
    TEnvironment const& fEnvironment;
    shared_ptr<const PROPOSAL::EnergyCutSettings> cut;

    static std::unordered_map<particles::Code, PROPOSAL::ParticleDef> particles;
    std::unordered_map<const NuclearComposition*, PROPOSAL::Medium> media;

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("p_rndm");

    bool CanInteract(particles::Code pcode) const noexcept {
      auto search = particles.find(pcode);
      if (search != particles.end()) return true;
      return false;
    };

    using calculator_t =
        tuple<PROPOSAL::SecondariesCalculator, unique_ptr<PROPOSAL::Interaction>,
              unique_ptr<PROPOSAL::Displacement>>;
    std::unordered_map<const NuclearComposition*, calculator_t> calculators;

    enum { SECONDARIES, INTERACTION, DISPLACEMENT };
    template <typename Particle>
    auto GetCalculator(Particle& vP) {
      auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calculators.find(&comp);
      if (calc_it != calculators.end()) return calc_it;
      auto cross =
          PROPOSAL::GetStdCrossSections(particles[vP.GetPID()], media[&comp], cut, true);
      auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(cross);
      auto [insert_it, success] = calculators.insert(
          {&comp, make_tuple(PROPOSAL::SecondariesCalculator(
                                 inter_types, particles[vP.GetPID()], media[&comp]),
                             PROPOSAL::make_interaction(cross, true),
                             PROPOSAL::make_displacement(cross, true))});
      return insert_it;
    }

  public:
    Interaction(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    void Init();

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle& p);
  };
} // namespace corsika::process::proposal
#endif
