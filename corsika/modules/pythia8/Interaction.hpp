/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/modules/pythia8/Pythia8.hpp>

#include <tuple>

namespace corsika::pythia8 {

  class Interaction : public InteractionProcess<Interaction>, public Pythia8::Pythia {

  public:
    Interaction(const bool print_listing = false);
    ~Interaction();

    void setStable(std::vector<Code> const&);
    void setUnstable(const Code);
    void setStable(const Code);

    bool isValidCoMEnergy(HEPEnergyType ecm) { return (10_GeV < ecm) && (ecm < 1_PeV); }

    bool canInteract(const Code);
    void configureLabFrameCollision(const Code, const Code, const HEPEnergyType);

    std::tuple<CrossSectionType, CrossSectionType> getCrossSection(
        const Code BeamId, const Code TargetId, const HEPEnergyType CoMenergy);

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&);

    /**
       In this function PYTHIA is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */
    template <typename TView>
    void doInteraction(TView&);

  private:
    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("pythia");
    Pythia8::SigmaTotal sigma_;
    const bool internalDecays_ = true;
    int count_ = 0;
    bool print_listing_ = false;
  };

} // namespace corsika::pythia8

#include <corsika/detail/modules/pythia8/Interaction.inl>
