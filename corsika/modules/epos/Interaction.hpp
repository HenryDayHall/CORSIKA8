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
    std::string data_path_;
    unsigned int count_ = 0;
    bool epos_listing_;
    
  public:
    Interaction(const std::string& dataPath = "", const bool epos_printout_on = false);
    ~Interaction();

    //! returns production and elastic cross section for hadrons in epos. Inputs are:
    //! CorsikaId of beam particle, CorsikaId of target particle, center-of-mass energy.
    //!  Allowed targets are: nuclei or single nucleons (p,n,hydrogen).
    std::tuple<CrossSectionType, CrossSectionType> getCrossSection(
        Code const, Code const, HEPEnergyType const) const;

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    /**
       In this function EPOSLHC is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */
    template <typename TSecondaries>
    void doInteraction(TSecondaries&);

    bool isValidCoMEnergy(HEPEnergyType const ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    //! eposlhc only accepts nuclei with 4<=A<=18 as targets, or protons aka Hydrogen or
    //! neutrons (p,n == nucleon)
    bool isValidTarget(Code const TargetId) const {
      return false;
    }
    
    void initialize_eposlhc_c7() const;
    void initialize_event_CoM(Code const, Code const, HEPEnergyType const) const;
    void initialize_event_Lab(Code const, Code const, HEPEnergyType const) const;
    void configure_particles(Code const, Code const) const;
    
  private:
    default_prng_type& RNG_ = RNGManager::getInstance().getRandomStream("epos");
    HEPEnergyType const minEnergyCoM_ = -10. * 1e9 * electronvolt;
    HEPEnergyType const maxEnergyCoM_ = -1.e6 * 1e9 * electronvolt;
    int const maxTargetMassNumber_ = 20;
    int const minNuclearTargetA_ = 4;
  };

} // namespace corsika::epos

#include <corsika/detail/modules/epos/Interaction.inl>
