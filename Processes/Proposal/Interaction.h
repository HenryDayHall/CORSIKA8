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

namespace corsika::process::proposal {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    shared_ptr<const PROPOSAL::EnergyCutSettings> cut;

    static std::unordered_map<particles::Code, PROPOSAL::ParticleDef> particles;
    std::unordered_map<const NuclearComposition*, PROPOSAL::Medium> media;

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("proposal");

    bool CanInteract(particles::Code pcode) const noexcept;

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
      return BuildCalculator(vP.GetPID(), comp);
    }

    auto BuildCalculator(particles::Code corsika_code, NuclearComposition const& comp) {
      auto medium = media.at(&comp);
      if (corsika_code == particles::Code::Gamma) {
          std::cout << "Build gamma tables" << std::endl;
        auto cross =
            GetStdCrossSections(PROPOSAL::GammaDef(), media.at(&comp), cut, true);
        auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(cross);
        auto [insert_it, success] = calculators.insert(
            {&comp, make_tuple(PROPOSAL::SecondariesCalculator(
                                   inter_types, PROPOSAL::GammaDef(), media[&comp]),
                               PROPOSAL::make_interaction(cross, true),
                               PROPOSAL::make_displacement(cross, true))});
        return insert_it;
      }
      if (corsika_code == particles::Code::Electron) {
          std::cout << "Build electron tables" << std::endl;
        auto cross =
            GetStdCrossSections(PROPOSAL::EMinusDef(), media.at(&comp), cut, true);
        auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(cross);
        auto [insert_it, success] = calculators.insert(
            {&comp, make_tuple(PROPOSAL::SecondariesCalculator(
                                   inter_types, PROPOSAL::EMinusDef(), media[&comp]),
                               PROPOSAL::make_interaction(cross, true),
                               PROPOSAL::make_displacement(cross, true))});
        return insert_it;
      }
      if (corsika_code == particles::Code::Positron) {
          std::cout << "Build positron tables" << std::endl;
        auto cross =
            GetStdCrossSections(PROPOSAL::EPlusDef(), media.at(&comp), cut, true);
        auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(cross);
        auto [insert_it, success] = calculators.insert(
            {&comp, make_tuple(PROPOSAL::SecondariesCalculator(
                                   inter_types, PROPOSAL::EPlusDef(), media[&comp]),
                               PROPOSAL::make_interaction(cross, true),
                               PROPOSAL::make_displacement(cross, true))});
        return insert_it;
      }
    } // namespace corsika::process::proposal

  public:
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    void Init();

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const& p);
  }; // namespace corsika::process::proposal
} // namespace corsika::process::proposal
#endif
