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

using namespace corsika::environment;
using namespace corsika::process::particle_cut;

namespace corsika::process::proposal {

  template <class TEnvironment>
  class Interaction : public corsika::process::InteractionProcess<Interaction<TEnvironment>> {

  private:
    TEnvironment const& fEnvironment;
    ParticleCut const& pCut;

  public:
    Interaction(TEnvironment const& env, ParticleCut const& cut);

    // ~Interaction;

    // template <typename Particle>
    // corsika::process::EProcessReturn DoInteraction(Particle&);

    // template <typename TParticle>
    // corsika::units::si::GrammageType GetInteractionLength(TParticle& p);

  };

} // namespace corsika::process
#endif
