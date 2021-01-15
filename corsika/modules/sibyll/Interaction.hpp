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
    bool isValidTarget(Code const TargetId) const {
      return is_nucleus(TargetId) && (get_nucleus_A(TargetId) < maxTargetMassNumber_);
    }

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
    void setStable(std::vector<Code> const&);
    void setUnstable(std::vector<Code> const&);

    void setUnstable(Code const);
    void setStable(Code const);
    void setAllUnstable();
    void setAllStable();

    int getMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    HEPEnergyType getMinEnergyCoM() const { return minEnergyCoM_; }
    HEPEnergyType getMaxEnergyCoM() const { return maxEnergyCoM_; }

    default_prng_type& RNG_ = RNGManager::getInstance().getRandomStream("sibyll");
    const bool internalDecays_ = true;
    const HEPEnergyType minEnergyCoM_ = 10. * 1e9 * electronvolt;
    const HEPEnergyType maxEnergyCoM_ = 1.e6 * 1e9 * electronvolt;
    const int maxTargetMassNumber_ = 18;

    // data members
    int count_ = 0;
    int nucCount_ = 0;
    bool sibyll_listing_;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/Interaction.inl>
