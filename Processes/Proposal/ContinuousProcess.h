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
#define _corsika_process_proposal_interaction_h_

#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <unordered_map>
#include "PROPOSAL/PROPOSAL.h"

using std::unordered_map;

using namespace corsika::environment;

using CORSIKA_ParticleCut = corsika::process::particle_cut::ParticleCut;

namespace corsika::process::proposal {

  class ContinuousProcess
      : public corsika::process::ContinuousProcess<ContinuousProcess> {
  private:
    shared_ptr<const PROPOSAL::EnergyCutSettings> cut;

    static unordered_map<particles::Code, PROPOSAL::ParticleDef> particles;
    unordered_map<const NuclearComposition*, PROPOSAL::Medium> media;

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("proposal");

    bool CanInteract(particles::Code pcode) const noexcept;

    struct interaction_hash {
        size_t operator()(const std::pair<const NuclearComposition*, particles::Code>& p) const
        {
            auto hash1 = std::hash<const NuclearComposition*>{}(p.first);
            auto hash2 = std::hash<particles::Code>{}(p.second);
            return hash1 ^ hash2;
        }
    };

    unordered_map<std::pair<const NuclearComposition*, particles::Code>, unique_ptr<PROPOSAL::Displacement>, interaction_hash> calc;

    auto BuildCalculator(particles::Code corsika_code, NuclearComposition const& comp) {
        auto medium = media.at(&comp);
        if (corsika_code == particles::Code::Gamma) {
            auto cross = GetStdCrossSections(PROPOSAL::GammaDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
                    calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::Electron) {
            auto cross = GetStdCrossSections(PROPOSAL::EMinusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::Positron) {
            auto cross = GetStdCrossSections(PROPOSAL::EPlusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::MuMinus) {
            auto cross = GetStdCrossSections(PROPOSAL::MuMinusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::MuPlus) {
            auto cross = GetStdCrossSections(PROPOSAL::MuPlusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::TauMinus) {
            auto cross = GetStdCrossSections(PROPOSAL::TauMinusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        if (corsika_code == particles::Code::TauPlus) {
            auto cross = GetStdCrossSections(PROPOSAL::TauPlusDef(), media.at(&comp), cut, true);
            auto [insert_it, success] =
            calc.insert({std::make_pair(&comp, corsika_code), PROPOSAL::make_displacement(cross, true)});
            return insert_it;
        }
        throw std::runtime_error("PROPOSAL could not find corresponding builder");
    }
    
    template <typename Particle>
    auto GetCalculator(Particle& vP) {
      auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calc.find(std::make_pair(&comp, vP.GetPID()));
      if (calc_it != calc.end()) return calc_it;
      return BuildCalculator(vP.GetPID(), comp);
    }

    units::si::HEPEnergyType TotalEnergyLoss(setup::Stack::ParticleType const&,
                                             const units::si::GrammageType);

  public:
    template <typename TEnvironment>
    ContinuousProcess(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    void Init();

    template <typename Particle, typename Track>
    EProcessReturn DoContinuous(Particle&, Track const&) ;

    template <typename Particle, typename Track>
    units::si::LengthType MaxStepLength(Particle const& p, Track const& track) ;
  };
} // namespace corsika::process::proposal

#endif
