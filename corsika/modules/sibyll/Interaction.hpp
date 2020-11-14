/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Random.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <tuple>

namespace corsika::sibyll {

  class Interaction : public corsika::InteractionProcess<Interaction> {

    int count_ = 0;
    int nucCount_ = 0;
    bool initialized_ = false;

  public:
    Interaction();
    ~Interaction();

    void Init() {}

    void SetStable(std::vector<corsika::Code> const&);
    void SetUnstable(std::vector<corsika::Code> const&);

    void SetUnstable(const corsika::Code);
    void SetStable(const corsika::Code);
    void SetAllUnstable();
    void SetAllStable();

    bool WasInitialized() { return initialized_; }
    bool IsValidCoMEnergy(HEPEnergyType ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    int GetMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    HEPEnergyType GetMinEnergyCoM() const { return minEnergyCoM_; }
    HEPEnergyType GetMaxEnergyCoM() const { return maxEnergyCoM_; }
    bool IsValidTarget(corsika::Code TargetId) const {
      return (corsika::GetNucleusA(TargetId) < maxTargetMassNumber_) &&
             corsika::IsNucleus(TargetId);
    }

    std::tuple<CrossSectionType, CrossSectionType> GetCrossSection(
        const corsika::Code, const corsika::Code, const HEPEnergyType) const;

    template <typename TParticle>
    GrammageType GetInteractionLength(TParticle const&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::default_prng_type& RNG_ = corsika::RNGManager::getInstance().getRandomStream("s_rndm");
    // FOR NOW keep trackedParticles private, could be configurable
    std::vector<corsika::Code> const trackedParticles_ = {
        corsika::Code::PiPlus,    corsika::Code::PiMinus,    corsika::Code::Pi0,
        corsika::Code::KMinus,    corsika::Code::KPlus,      corsika::Code::K0Long,
        corsika::Code::K0Short,   corsika::Code::SigmaPlus,  corsika::Code::Sigma0,
        corsika::Code::Sigma0Bar, corsika::Code::SigmaMinus, corsika::Code::Lambda0,
        corsika::Code::Xi0,       corsika::Code::XiMinus,    corsika::Code::OmegaMinus,
        corsika::Code::DPlus,     corsika::Code::DMinus,     corsika::Code::D0,
        corsika::Code::MuMinus,   corsika::Code::MuPlus,     corsika::Code::D0Bar};
    const bool internalDecays_ = true;
    const HEPEnergyType minEnergyCoM_ = 10. * 1e9 * electronvolt;
    const HEPEnergyType maxEnergyCoM_ = 1.e6 * 1e9 * electronvolt;
    const int maxTargetMassNumber_ = 18;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/Interaction.inl>
