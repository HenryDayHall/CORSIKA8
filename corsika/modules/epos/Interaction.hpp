/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <tuple>

namespace corsika::epos {

  class Interaction : public InteractionProcess<Interaction> {

  public:
    Interaction();
    ~Interaction();

    //! returns production and elastic cross section for hadrons in sibyll. Inputs are:
    //! CorsikaId of beam particle, CorsikaId of target particle and center-of-mass
    //! energy. Allowed targets are: nuclei or single nucleons (p,n,hydrogen).
    CrossSectionType getCrossSection(Code const, Code const, HEPEnergyType const) const;

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */
    template <typename TSecondaries>
    void doInteraction(TSecondaries&);

  private:
    default_prng_type& RNG_ = RNGManager::getInstance().getRandomStream("epos");
    unsigned int count_;
  };

} // namespace corsika::epos

#include <corsika/detail/modules/epos/Interaction.inl>
