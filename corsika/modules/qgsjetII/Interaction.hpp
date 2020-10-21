/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>

#include <string>

namespace corsika::qgsjetII {

  class Interaction : public corsika::InteractionProcess<Interaction> {

    std::string data_path_;
    int count_ = 0;
    bool initialized_ = false;

  public:
    Interaction(const std::string& dataPath = "");
    ~Interaction();

    void Init();

    bool WasInitialized() { return initialized_; }
    int GetMaxTargetMassNumber() const { return maxMassNumber_; }
    bool IsValidTarget(corsika::Code TargetId) const {
      return (corsika::GetNucleusA(TargetId) < maxMassNumber_) &&
             corsika::IsNucleus(TargetId);
    }

    corsika::units::si::CrossSectionType GetCrossSection(
        const corsika::Code, const corsika::Code, const corsika::units::si::HEPEnergyType,
        const unsigned int Abeam = 0, const unsigned int Atarget = 0) const;

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const&) const;

    /**
       In this function QGSJETII is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::RNG& fRNG = corsika::RNGManager::GetInstance().GetRandomStream("qgran");
    const int maxMassNumber_ = 208;
  };

} // namespace corsika::qgsjetII

#include <corsika/detail/modules/qgsjetII/Interaction.inl>
