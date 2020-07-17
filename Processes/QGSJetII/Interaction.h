/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <string>

namespace corsika::process::qgsjetII {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    std::string data_path_;
    int count_ = 0;
    bool initialized_ = false;
    QgsjetIIHadronType alternate_ =
        QgsjetIIHadronType::PiPlusType; // for pi0, rho0 projectiles

  public:
    Interaction(const std::string& dataPath = "");
    ~Interaction();

    void Init();

    bool WasInitialized() { return initialized_; }
    int GetMaxTargetMassNumber() const { return maxMassNumber_; }
    bool IsValidTarget(corsika::particles::Code TargetId) const {
      return (corsika::particles::GetNucleusA(TargetId) < maxMassNumber_) &&
             corsika::particles::IsNucleus(TargetId);
    }

    corsika::units::si::CrossSectionType GetCrossSection(
        const corsika::particles::Code, const corsika::particles::Code,
        const corsika::units::si::HEPEnergyType, const unsigned int Abeam = 0,
        const unsigned int Atarget = 0) const;

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const&) const;

    /**
       In this function QGSJETII is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::process::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::random::RNG& rng_ =
        corsika::random::RNGManager::GetInstance().GetRandomStream("qgran");
    static constexpr int maxMassNumber_ = 208;
  };

} // namespace corsika::process::qgsjetII
