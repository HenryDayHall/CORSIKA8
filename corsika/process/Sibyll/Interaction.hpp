/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <tuple>

namespace corsika::sibyll {

  class Interaction : public corsika::InteractionProcess<Interaction> {

    int count_ = 0;
    int nucCount_ = 0;
    static bool initialized_; ///! flag to assure init is done only once
    bool sibyll_listing_;

  public:
    Interaction(const bool sibyll_printout_on = false);
    ~Interaction();

    void Init();

    void SetStable(std::vector<particles::Code> const&);
    void SetUnstable(std::vector<particles::Code> const&);

    void SetUnstable(const corsika::Code);
    void SetStable(const corsika::Code);
    void SetAllUnstable();
    void SetAllStable();

    static bool WasInitialized() { return initialized_; }
    bool IsValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) const {
      return (minEnergyCoM_ <= ecm) && (ecm <= maxEnergyCoM_);
    }
    int GetMaxTargetMassNumber() const { return maxTargetMassNumber_; }
    corsika::units::si::HEPEnergyType GetMinEnergyCoM() const { return minEnergyCoM_; }
    corsika::units::si::HEPEnergyType GetMaxEnergyCoM() const { return maxEnergyCoM_; }
    bool IsValidTarget(corsika::Code TargetId) const {
      return (corsika::GetNucleusA(TargetId) < maxTargetMassNumber_) &&
             corsika::IsNucleus(TargetId);
    }

    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::Code, const corsika::Code,
                    const corsika::units::si::HEPEnergyType) const;

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(const TParticle&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::RNG& RNG_ =
        corsika::RNGManager::GetInstance().GetRandomStream("s_rndm");
    // FOR NOW keep trackedParticles private, could be configurable
    std::vector<particles::Code> const trackedParticles_ = {
        particles::Code::PiPlus,     particles::Code::PiMinus,
        particles::Code::Pi0,        particles::Code::KMinus,
        particles::Code::KPlus,      particles::Code::K0Long,
        particles::Code::K0Short,    particles::Code::SigmaPlus,
        particles::Code::Sigma0,     particles::Code::Sigma0Bar,
        particles::Code::SigmaMinus, particles::Code::Lambda0,
        particles::Code::Xi0,        particles::Code::XiMinus,
        particles::Code::OmegaMinus, particles::Code::DPlus,
        particles::Code::DMinus,     particles::Code::D0,
        particles::Code::MuMinus,    particles::Code::MuPlus,
        particles::Code::D0Bar};
    const bool internalDecays_ = true;
    const corsika::units::si::HEPEnergyType minEnergyCoM_ =
        10. * 1e9 * corsika::units::si::electronvolt;
    const corsika::units::si::HEPEnergyType maxEnergyCoM_ =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
    const int maxTargetMassNumber_ = 18;
  };

} // namespace corsika::sibyll

#endif
