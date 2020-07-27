/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/sibyll/sibyll2.3d.h>
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

    void Init();

    void SetAllStable();

    bool WasInitialized() { return initialized_; }
    bool IsValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    int GetMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    corsika::units::si::HEPEnergyType GetMinEnergyCoM() const { return minEnergyCoM_; }
    corsika::units::si::HEPEnergyType GetMaxEnergyCoM() const { return maxEnergyCoM_; }
    double get_relative_precision_momentum() const {
      if (get_nwounded() == 1)
        return precision_momentum_single_;
      else
        return precision_momentum_ * get_nwounded();
    }
    double get_relative_precision_energy() const {
      if (get_nwounded() == 1)
        return precision_energy_single_;
      else
        return precision_energy_ * get_nwounded();
    }
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
    /*
       for interactions with a single target nucleon energy and momentum conservation
       are fullfilled
       for more than one target nucleon, conservation is only approximately true in the
       lab frame in addition there seems to be a bug in sibyll that leads to violation of
       momentum conservation already in the nuc-nuc frame. see Issue #272

       https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/issues/272
     */
    const double precision_energy_single_ = 1.e-8;
    const double precision_momentum_single_ = 1.e-8;
    const double precision_energy_ = 2.e-2;
    const double precision_momentum_ = 5.e-2;
  };

} // namespace corsika::process::sibyll
