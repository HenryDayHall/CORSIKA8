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

#include <PROPOSAL/PROPOSAL.h>


namespace corsika::process::proposal {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {
  private:


  public:
    template <typename Particle>
    EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle& p);

  }
} // namespace corsika::process
