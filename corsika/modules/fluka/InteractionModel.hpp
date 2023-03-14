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
#include <memory>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>

namespace corsika::fluka {
  /**
   * This class exposes the (hadronic) interactions of FLUKA. FLUKA needs to be
   * initialized with a predefined set of target materials and a flag describing the type
   * of interactions (elastic, inelastic, electromagnetic dissociation). Currently, only
   * inelastic events are supported.
   *
   */
  class InteractionModel {
  public:
    /**
     * Create a new InteractionModel. The FLUKA materials are collected from the elements
     * present in the environment. Each element is its own FLUKA material, no FLUKA
     * compounds are used.
     */
    template <typename TEnvironment>
    InteractionModel(TEnvironment const&);

    //! Return the cross-section of a given combination of projectile/target.
    CrossSectionType getCrossSection(Code projectileId, Code targetId,
                                     FourMomentum const& projectileP4,
                                     FourMomentum const& targetP4) const;

    bool isValid(Code projectileID, Code targetID, HEPEnergyType sqrtS) const;
    bool isValid(Code projectileID, int material, HEPEnergyType sqrtS) const;

    //! convert target Code to FLUKA material number
    int getMaterialIndex(Code targetID) const;

    template <typename TSecondaryView>
    void doInteraction(TSecondaryView& view, Code const projectileId, Code const targetId,
                       FourMomentum const& projectileP4, FourMomentum const& targetP4);

  private:
    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("fluka");
    std::vector<std::pair<Code, int>> const
        materials_; //!< map target Code to FLUKA material no.
    std::shared_ptr<spdlog::logger> logger_ = get_logger("corsika_FLUKA_Interaction");
    std::unique_ptr<double[]> cumsgx_; //!< dump for evtxyz cumsg*, never read again

    template <typename TEnvironment>
    static std::vector<std::pair<Code, int>> genFlukaMaterials(TEnvironment const&);
  };

  inline static int const iflxyz_ = 1;
} // namespace corsika::fluka

#include <corsika/detail/modules/fluka/InteractionModel.inl>
