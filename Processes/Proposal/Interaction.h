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
#include <unordered_map>
#include "PROPOSAL/PROPOSAL.h"

using namespace corsika::environment;

using CORSIKA_ParticleCut = corsika::process::particle_cut::ParticleCut;
using std::make_pair;
using std::make_tuple;

namespace corsika::process::proposal {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    shared_ptr<const PROPOSAL::EnergyCutSettings> cut;

    static std::unordered_map<particles::Code, PROPOSAL::ParticleDef> particles;
    std::unordered_map<const NuclearComposition*, PROPOSAL::Medium> media;

    corsika::random::RNG& fRNG;

    bool CanInteract(particles::Code pcode) const noexcept;

    using calculator_t = tuple<unique_ptr<PROPOSAL::SecondariesCalculator>,
                               unique_ptr<PROPOSAL::Interaction>>;
    using calc_key_t = std::pair<const NuclearComposition*, particles::Code>;

    struct interaction_hash {
      size_t operator()(const calc_key_t& p) const {
        return std::hash<const NuclearComposition*>{}(p.first) ^
               std::hash<particles::Code>{}(p.second);
      }
    };

    std::unordered_map<calc_key_t, calculator_t, interaction_hash> calculators;

    template <typename Particle>
    auto BuildCalculator(particles::Code corsika_code, Particle p_def,
                         NuclearComposition const& comp) {
      auto cross = GetStdCrossSections(p_def, media.at(&comp), cut, true);
      auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(cross);
      auto [insert_it, success] = calculators.insert(
          {make_pair(&comp, corsika_code),
           make_tuple(PROPOSAL::make_secondaries(inter_types, p_def, media.at(&comp)),
                      PROPOSAL::make_interaction(cross, true))});
      return insert_it;
    }

    auto BuildCalculator(particles::Code corsika_code, NuclearComposition const& comp) {
      if (corsika_code == particles::Code::Gamma)
        return BuildCalculator(particles::Code::Gamma, PROPOSAL::GammaDef(), comp);
      if (corsika_code == particles::Code::Electron)
        return BuildCalculator(particles::Code::Electron, PROPOSAL::EMinusDef(), comp);
      if (corsika_code == particles::Code::Positron)
        return BuildCalculator(particles::Code::Positron, PROPOSAL::EPlusDef(), comp);
      if (corsika_code == particles::Code::MuMinus)
        return BuildCalculator(particles::Code::MuMinus, PROPOSAL::MuMinusDef(), comp);
      if (corsika_code == particles::Code::MuPlus)
        return BuildCalculator(particles::Code::MuPlus, PROPOSAL::MuPlusDef(), comp);
      if (corsika_code == particles::Code::TauMinus)
        return BuildCalculator(particles::Code::TauMinus, PROPOSAL::TauMinusDef(), comp);
      if (corsika_code == particles::Code::TauPlus)
        return BuildCalculator(particles::Code::TauPlus, PROPOSAL::TauPlusDef(), comp);
      throw std::runtime_error("PROPOSAL could not find corresponding builder");
    }

    enum { SECONDARIES, INTERACTION };
    template <typename Particle>
    auto GetCalculator(Particle& vP) {
      auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calculators.find(make_pair(&comp, vP.GetPID()));
      if (calc_it != calculators.end()) return calc_it;
      return BuildCalculator(vP.GetPID(), comp);
    }

  public:
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    void Init() {};

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const& p);
  }; // namespace corsika::process::proposal
} // namespace corsika::process::proposal
#endif
