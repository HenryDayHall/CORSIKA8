/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_qgsjetII_interaction_h_
#define _corsika_process_qgsjetII_interaction_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <tuple>
#include <string>

namespace corsika::process::qgsjetII {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    std::string data_path_;
    int fCount = 0;
    bool fInitialized = false;

  public:
    Interaction(const std::string& dataPath="");
    ~Interaction();

    void Init();

    bool WasInitialized() { return fInitialized; }
    bool IsValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) const {
      return (fMinEnergyCoM <= ecm) && (ecm <= fMaxEnergyCoM);
    }
    int GetMaxTargetMassNumber() const { return fMaxMassNumber; }
    corsika::units::si::HEPEnergyType GetMinEnergyCoM() const { return fMinEnergyCoM; }
    corsika::units::si::HEPEnergyType GetMaxEnergyCoM() const { return fMaxEnergyCoM; }
    bool IsValidTarget(corsika::particles::Code TargetId) const {
      return (corsika::particles::GetNucleusA(TargetId) < fMaxMassNumber) &&
             corsika::particles::IsNucleus(TargetId);
    }

    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::particles::Code, const corsika::particles::Code,
                    const corsika::units::si::HEPEnergyType,
		    const unsigned int Abeam=0, 
		    const unsigned int Atarget=0) const;

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle const&) const;

    /**
       In this function QGSJETII is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::process::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
    // FOR NOW keep trackedParticles private, could be configurable
    std::vector<particles::Code> const fTrackedParticles = {
        particles::Code::PiPlus,     particles::Code::PiMinus,
        particles::Code::Pi0,        particles::Code::KMinus,
        particles::Code::KPlus,      particles::Code::K0Long,
        particles::Code::K0Short,    particles::Code::SigmaPlus,
        particles::Code::SigmaMinus, particles::Code::Lambda0,
        particles::Code::Xi0,        particles::Code::XiMinus,
        particles::Code::OmegaMinus, particles::Code::DPlus,
        particles::Code::DMinus,     particles::Code::D0,
        particles::Code::MuMinus,    particles::Code::MuPlus,
        particles::Code::D0Bar};
    const corsika::units::si::HEPEnergyType fMinEnergyCoM =
        10. * 1e9 * corsika::units::si::electronvolt;
    const corsika::units::si::HEPEnergyType fMaxEnergyCoM =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
    const int fMaxMassNumber = 208;
  };

} // namespace corsika::process::qgsjetII

#endif
