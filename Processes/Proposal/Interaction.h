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

#include "PROPOSAL/PROPOSAL.h"
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/environment/Environment.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/random/RNGManager.h>
#include <random>

using namespace corsika::environment;
using namespace corsika::process::particle_cut;

namespace corsika::process::proposal {

  template <class TEnvironment>
  class Interaction : public corsika::process::InteractionProcess<Interaction<TEnvironment>> {

  private:
    TEnvironment const& fEnvironment;
    ParticleCut const& pCut;
    // Initializing of uniform_real_distribution class
    // double min{0.0};
    // double max{1.0};
    // std::uniform_real_distribution<double> rnd_uniform(min,max);

    std::map<particles::Code, std::unique_ptr<PROPOSAL::UtilityInterpolantInteraction>> corsika_particle_to_utility_map;
	std::map<particles::Code, std::unique_ptr<PROPOSAL::UtilityInterpolantDisplacement>> corsika_particle_to_displacement_map;

    corsika::random::RNG& fRNG = corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

    std::map<std::string, particles::Code> const convert_proposal_particle_name_to_corsika_code = {
        {"Gamma", particles::Code::Gamma},
        {"EMinus", particles::Code::Electron},
        {"EPlus", particles::Code::Positron},
        {"MuMinu", particles::Code::MuMinus},
        {"MuPlus", particles::Code::MuPlus},
        {"TauPlus", particles::Code::TauPlus},
        {"TauMinus",particles::Code::TauMinus},
    };

    std::map<particles::Code, PROPOSAL::ParticleDef> const convert_corsika_code_to_proposal_particle_def = {
        {particles::Code::Gamma, PROPOSAL::GammaDef::Get()},
        {particles::Code::Electron, PROPOSAL::EMinusDef::Get()},
        {particles::Code::Positron, PROPOSAL::EPlusDef::Get()},
        {particles::Code::MuMinus, PROPOSAL::MuMinusDef::Get()},
        {particles::Code::MuPlus, PROPOSAL::MuPlusDef::Get()},
        {particles::Code::TauPlus, PROPOSAL::TauPlusDef::Get()},
        {particles::Code::TauMinus, PROPOSAL::TauMinusDef::Get()},
    };

    // fTrackedParticles[proposal_particle.GetName()] return  particle::Code

    bool IsTracked(particles::Code pcode) {
        for (auto i : convert_corsika_code_to_proposal_particle_def) if (i.first==pcode) return true;
        return false;
    };


  public:
    Interaction(TEnvironment const& env, ParticleCut const& cut);

    // ~Interaction;

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle& p);

  };

} // namespace corsika::process
#endif
