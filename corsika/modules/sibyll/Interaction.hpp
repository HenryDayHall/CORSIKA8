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

namespace corsika::sibyll {

  class Interaction : public InteractionProcess<Interaction> {

  public:
    Interaction(bool const sibyll_printout_on = false);
    ~Interaction();

    bool isValidCoMEnergy(HEPEnergyType const ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    //! sibyll only accepts nuclei with 4<=A<=18 as targets, or protons aka Hydrogen or
    //! neutrons (p,n == nucleon)
    bool isValidTarget(Code const TargetId) const {
      return (is_nucleus(TargetId) && (get_nucleus_A(TargetId) >= minNuclearTargetA_) &&
              (get_nucleus_A(TargetId) < maxTargetMassNumber_)) ||
             (TargetId == Code::Proton || TargetId == Code::Hydrogen ||
              TargetId == Code::Neutron);
    }

    //! returns production and elastic cross section for hadrons in sibyll. Inputs are:
    //! CorsikaId of beam particle, CorsikaId of target particle and center-of-mass
    //! energy. Allowed targets are: nuclei or single nucleons (p,n,hydrogen).
    std::tuple<CrossSectionType, CrossSectionType> getCrossSection(
        Code const, Code const, HEPEnergyType const) const;

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TSecondaries>
    void doInteraction(TSecondaries&);

  private:
    int getMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    HEPEnergyType getMinEnergyCoM() const { return minEnergyCoM_; }
    HEPEnergyType getMaxEnergyCoM() const { return maxEnergyCoM_; }

    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("sibyll");
    const HEPEnergyType minEnergyCoM_ = 10. * 1e9 * electronvolt;
    const HEPEnergyType maxEnergyCoM_ = 1.e6 * 1e9 * electronvolt;
    const int maxTargetMassNumber_ = 18;
    const int minNuclearTargetA_ = 4;

    // data members
    int count_ = 0;
    int nucCount_ = 0;
    bool sibyll_listing_;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/Interaction.inl>
