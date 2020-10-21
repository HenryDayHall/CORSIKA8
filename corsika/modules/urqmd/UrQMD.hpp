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

  class UrQMD : public corsika::InteractionProcess<UrQMD> {
  public:
    UrQMD();
    void Init() {}
    GrammageType GetInteractionLength(corsika::setup::Stack::StackIterator&) const;

    template <typename TParticle>
    CrossSectionType GetCrossSection(TParticle const&, corsika::Code) const;

    corsika::EProcessReturn DoInteraction(corsika::setup::StackView::StackIterator&);

    bool CanInteract(corsika::Code) const;

  private:
    static CrossSectionType GetCrossSection(corsika::Code, corsika::Code, HEPEnergyType,
                                            int);
    corsika::RNG& fRNG = corsika::RNGManager::GetInstance().GetRandomStream("UrQMD");

    std::uniform_int_distribution<int> fBooleanDist{0, 1};
  };

  /**
   * convert CORSIKA code to UrQMD code tuple
   *
   * In the current implementation a detour via the PDG code is made.
   */
  std::pair<int, int> ConvertToUrQMD(corsika::Code);
  corsika::Code ConvertFromUrQMD(int vItyp, int vIso3);

} // namespace corsika::urqmd

#include <corsika/detail/modules/urqmd/UrQMD.inl>
