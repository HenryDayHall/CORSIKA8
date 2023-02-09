/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <vector>
#include <utility>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika::fluka {
  class InteractionModel {
  public:
    template <typename TEnvironment>
    InteractionModel(TEnvironment const&);

    CrossSectionType getCrossSection(Code projectileId, Code targetId,
                                     FourMomentum const& projectileP4,
                                     FourMomentum const& targetP4) const;

    bool isValid(Code projectileID, Code targetID, HEPEnergyType sqrtS) const;
    bool isValid(Code projectileID, int material, HEPEnergyType sqrtS) const;

    int getMaterialIndex(Code targetID) const;

    template <typename TSecondaryView>
    void doInteraction(TSecondaryView& view, Code const projectileId, Code const targetId,
                       FourMomentum const& projectileP4, FourMomentum const& targetP4);

  private:
    std::vector<std::pair<Code, int>> const
        materials_; //!< map target Code to FLUKA material no.

    template <typename TEnvironment>
    static std::vector<std::pair<Code, int>> genFlukaMaterials(TEnvironment const&);
    // TODO: random number stream
  };

  inline static int const iflxyz_ = 1;
} // namespace corsika::fluka

#include <corsika/detail/modules/fluka/InteractionModel.inl>
