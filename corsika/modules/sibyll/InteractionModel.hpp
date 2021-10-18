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
#include <corsika/framework/utility/COMBoost.hpp>
#include <tuple>

namespace corsika::sibyll {

  /**
   * @brief sibyll::InteractionModel provides the SIBYLL proton-nucleus interaction model.
   *
   * This is a TModel argument for InteractionProcess<TModel>.
   */

  class InteractionModel {

  public:
    InteractionModel();
    ~InteractionModel();

    /**
     * @brief Set the Verbose flag.
     *
     * If flag is true, SIBYLL will printout additional secondary particle information
     * lists, etc.
     *
     * @param flag to switch.
     */
    void setVerbose(bool const flag);

    /**
     * @brief evaluated validity of collision system.
     *
     * sibyll only accepts nuclei with 4<=A<=18 as targets, or protons aka Hydrogen or
     * neutrons (p,n == nucleon).
     */
    void constexpr isValid(Code const projectileId, Code const targetId,
                           HEPEnergyType const sqrtSnn) const;

    /**
     * @brief returns inelastic AND elastic cross sections.
     *
     * These cross sections must correspond to the process described in doInteraction
     * AND elastic scattering (sigma_tot = sigma_inel + sigma_el). Allowed targets are:
     * nuclei or single nucleons (p,n,hydrogen). This "InelEla" method is used since
     * Sibyll must be useful inside the NuclearInteraction model, which requires that.
     *
     * @param projectile is the Code of the projectile
     * @param target is the Code of the target
     * @param sqrtSnn is the center-of-mass energy (per nucleon pair)
     * @param Aprojectil is the mass number of the projectils, if it is a nucleus
     * @param Atarget is the mass number of the target, if it is a nucleus
     *
     * @return a tuple of: inelastic cross section, elastic cross section
     */
    std::tuple<CrossSectionType, CrossSectionType> getCrossSectionInelEla(
        Code const projectile, Code const target, FourMomentum const& projectileP4,
        FourMomentum const& targetP4) const;

    /**
     * @brief returns inelastic (production) cross section.
     *
     * This cross section must correspond to the process described in doInteraction.
     * Allowed targets are: nuclei or single nucleons (p,n,hydrogen).
     *
     * @param projectile is the Code of the projectile
     * @param target is the Code of the target
     * @param sqrtSnn is the center-of-mass energy (per nucleon pair)
     * @param Aprojectil is the mass number of the projectils, if it is a nucleus
     * @param Atarget is the mass number of the target, if it is a nucleus
     *
     * @return inelastic cross section
     * elastic cross section
     */
    CrossSectionType getCrossSection(Code const projectile, Code const target,
                                     FourMomentum const& projectileP4,
                                     FourMomentum const& targetP4) const {
      return std::get<0>(
          getCrossSectionInelEla(projectile, target, projectileP4, targetP4));
    }

    /**
     * In this function SIBYLL is called to produce one event. The
     * event is copied (and boosted) into the shower lab frame.
     */

    template <typename TSecondaries>
    void doInteraction(TSecondaries&, Code const projectile, Code const target,
                       FourMomentum const& projectileP4, FourMomentum const& targetP4);

  private:
    HEPEnergyType constexpr getMinEnergyCoM() const { return minEnergyCoM_; }
    HEPEnergyType constexpr getMaxEnergyCoM() const { return maxEnergyCoM_; }

    // hard model limits
    static HEPEnergyType constexpr minEnergyCoM_ = 10. * 1e9 * electronvolt;
    static HEPEnergyType constexpr maxEnergyCoM_ = 1.e6 * 1e9 * electronvolt;
    static unsigned int constexpr maxTargetMassNumber_ = 18;
    static unsigned int constexpr minNuclearTargetA_ = 4;

    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("sibyll");

    // data members
    int count_ = 0;
    int nucCount_ = 0;
    bool sibyll_listing_;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/InteractionModel.inl>
