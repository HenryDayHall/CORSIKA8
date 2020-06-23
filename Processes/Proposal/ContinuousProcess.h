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

    unordered_map<const NuclearComposition*, unique_ptr<PROPOSAL::Displacement>> calc;

    template <typename Particle>
    auto GetCalculator(Particle& vP) {
      auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calc.find(&comp);
      if (calc_it != calc.end()) return calc_it;
      auto cross =
          PROPOSAL::GetStdCrossSections(particles[vP.GetPID()], media[&comp], cut, true);
      auto [insert_it, success] =
          calc.insert({&comp, PROPOSAL::make_displacement(cross, true)});
      return insert_it;
    }

    units::si::HEPEnergyType TotalEnergyLoss(setup::Stack::ParticleType const&,
                                             const units::si::GrammageType);

  public:
    template <typename TEnvironment>
    ContinuousProcess(TEnvironment const& env, CORSIKA_ParticleCut const& cut);

    template <typename Particle, typename Track>
    EProcessReturn DoContinuous(Particle&, Track const&) ;

    template <typename Particle, typename Track>
    units::si::LengthType MaxStepLength(Particle const& p, Track const& track) ;
  };
} // namespace corsika::process::proposal

#endif
