/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <tuple>

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int count_ = 0;
    int nucCount_ = 0;
    bool initialized_ = false;

  public:
    Interaction();
    ~Interaction();

    void SetAllStable();

    bool WasInitialized() { return initialized_; }
    bool IsValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    int GetMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    corsika::units::si::HEPEnergyType GetMinEnergyCoM() const { return minEnergyCoM_; }
    corsika::units::si::HEPEnergyType GetMaxEnergyCoM() const { return maxEnergyCoM_; }
    bool IsValidTarget(corsika::particles::Code TargetId) const {
      return (corsika::particles::GetNucleusA(TargetId) < maxTargetMassNumber_) &&
             corsika::particles::IsNucleus(TargetId);
    }

    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::particles::Code, const corsika::particles::Code,
                    const corsika::units::si::HEPEnergyType) const;

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::process::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::random::RNG& RNG_ =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

    const corsika::units::si::HEPEnergyType minEnergyCoM_ =
        10. * 1e9 * corsika::units::si::electronvolt;
    const corsika::units::si::HEPEnergyType maxEnergyCoM_ =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
    const int maxTargetMassNumber_ = 18;
  };

} // namespace corsika::process::sibyll
