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
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <corsika/modules/urqmd/Random.hpp>

#include <array>
#include <utility>

namespace corsika::urqmd {

  class UrQMD : public InteractionProcess<UrQMD> {
  public:
    UrQMD();

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    template <typename TParticle>
    CrossSectionType getCrossSection(TParticle const&, Code) const;

    template <typename TView>
    void doInteraction(TView&);

    bool canInteract(Code) const;

  private:
    static CrossSectionType getCrossSection(Code, Code, HEPEnergyType, int);

    // data members
    default_prng_type& RNG_ = RNGManager::getInstance().getRandomStream("urqmd");

    std::uniform_int_distribution<int> booleanDist_{0, 1};
  };

  /**
   * convert CORSIKA code to UrQMD code tuple
   *
   * In the current implementation a detour via the PDG code is made.
   */
  std::pair<int, int> convertToUrQMD(Code);
  Code convertFromUrQMD(int vItyp, int vIso3);

} // namespace corsika::urqmd

#include <corsika/detail/modules/urqmd/UrQMD.inl>
