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
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <tuple>

namespace corsika::epos {

  class InteractionModel {

  public:
    InteractionModel(std::string const& dataPath = "",
                     bool const epos_printout_on = false);
    ~InteractionModel();

    //! returns production and elastic cross section for hadrons in epos. Inputs are:
    //! CorsikaId of beam particle, CorsikaId of target particle, center-of-mass energy.
    //!  Allowed targets are: nuclei or single nucleons (p,n,hydrogen). This routine
    //!  calculates the cross sections from scratch. Very slow!
    std::tuple<CrossSectionType, CrossSectionType> calcCrossSectionCoM(
        Code const, int const, int const, Code const, int const, int const,
        HEPEnergyType const) const;

    //! returns production and elastic cross section for hadrons in epos by reading
    //! pre-calculated tables from epos.
    std::tuple<CrossSectionType, CrossSectionType> readCrossSectionTableLab(
        Code const, int const, int const, Code const, HEPEnergyType const) const;

    //! returns production and elastic cross section. Allowed configurations are
    //! hadron-nucleon, hadron-nucleus and nucleus-nucleus. Inputs are particle id's mass
    //! and charge numbers and total energy in the lab.
    std::tuple<CrossSectionType, CrossSectionType> getCrossSectionInelEla(
        Code const projectileId, Code const targetId, FourMomentum const& projectileP4,
        FourMomentum const& targetP4) const;

    /**
     * Checks validity of projectile, target and energy combination.
     */
    bool isValid(Code const projectileId, Code const targetId,
                 HEPEnergyType const sqrtS) const;

    /**
     * Get the inelatic/production cross section.
     *
     * @param projectileId
     * @param targetId
     * @param projectileP4
     * @param targetP4
     * @return CrossSectionType
     */
    CrossSectionType getCrossSection(Code const projectileId, Code const targetId,
                                     FourMomentum const& projectileP4,
                                     FourMomentum const& targetP4) const {
      return std::get<0>(
          getCrossSectionInelEla(projectileId, targetId, projectileP4, targetP4));
    }

    /**
     * In this function EPOSLHC is called to produce one event. The
     * event is copied into the shower lab frame.
     */
    template <typename TSecondaries>
    void doInteraction(TSecondaries&, Code const projectileId, Code const targetId,
                       FourMomentum const& projectileP4, FourMomentum const& targetP4);

    void initialize() const;
    void initializeEventCoM(Code const, int const, int const, Code const, int const,
                            int const, HEPEnergyType const) const;
    void initializeEventLab(Code const, int const, int const, Code const, int const,
                            int const, HEPEnergyType const) const;
    void configureParticles(Code const, int const, int const, Code const, int const,
                            int const) const;
    void setParticlesStable() const;

  private:
    std::string data_path_;
    unsigned int count_ = 0;
    bool epos_listing_;

    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("epos");
    std::shared_ptr<spdlog::logger> logger_ = get_logger("corsika_epos_Interaction");
    HEPEnergyType const minEnergyCoM_ = 6 * 1e9 * electronvolt;
    HEPEnergyType const maxEnergyCoM_ = 2.e6 * 1e9 * electronvolt;
    static unsigned int constexpr maxTargetMassNumber_ = 20;
    static unsigned int constexpr minNuclearTargetA_ = 4;
  };

} // namespace corsika::epos

#include <corsika/detail/modules/epos/Interaction.inl>
