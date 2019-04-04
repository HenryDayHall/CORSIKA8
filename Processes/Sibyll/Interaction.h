
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_sibyll_interaction_h_
#define _corsika_process_sibyll_interaction_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <tuple>

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int fCount = 0;
    int fNucCount = 0;
    bool fInitialized = false;

  public:
    Interaction();
    ~Interaction();

    void Init();

    void SetParticleListStable(std::vector<particles::Code> const&);
    void SetUnstable(const corsika::particles::Code);
    void SetStable(const corsika::particles::Code);

    bool WasInitialized() { return fInitialized; }
    bool IsValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) const {
      return (fMinEnergyCoM <= ecm) && (ecm <= fMaxEnergyCoM);
    }
    int GetMaxTargetMassNumber() const { return fMaxTargetMassNumber; }
    corsika::units::si::HEPEnergyType GetMinEnergyCoM() const { return fMinEnergyCoM; }
    corsika::units::si::HEPEnergyType GetMaxEnergyCoM() const { return fMaxEnergyCoM; }
    bool IsValidTarget(corsika::particles::Code TargetId) const {
      return (corsika::particles::GetNucleusA(TargetId) < fMaxTargetMassNumber) &&
             corsika::particles::IsNucleus(TargetId);
    }

    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::particles::Code BeamId,
                    const corsika::particles::Code TargetId,
                    const corsika::units::si::HEPEnergyType CoMenergy) const;

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle&, Track&) const;

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle&, Stack&);

  private:
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

    const bool fInternalDecays = true;
    const corsika::units::si::HEPEnergyType fMinEnergyCoM =
        10. * 1e9 * corsika::units::si::electronvolt;
    const corsika::units::si::HEPEnergyType fMaxEnergyCoM =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
    const int fMaxTargetMassNumber = 18;
  };

} // namespace corsika::process::sibyll

#endif
