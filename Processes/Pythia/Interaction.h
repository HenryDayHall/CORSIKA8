/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_pythia_interaction_h_
#define _corsika_process_pythia_interaction_h_

#include <Pythia8/Pythia.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <tuple>

namespace corsika::process::pythia {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int fCount = 0;
    bool fInitialized = false;

  public:
    Interaction() {}
    ~Interaction();

    void Init();

    void SetParticleListStable(std::vector<particles::Code> const&);
    void SetUnstable(const corsika::particles::Code);
    void SetStable(const corsika::particles::Code);

    bool WasInitialized() { return fInitialized; }
    bool ValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) {
      using namespace corsika::units::si;
      return (10_GeV < ecm) && (ecm < 1_PeV);
    }

    bool CanInteract(const corsika::particles::Code);
    void ConfigureLabFrameCollision(const corsika::particles::Code,
                                    const corsika::particles::Code,
                                    const corsika::units::si::HEPEnergyType);
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::particles::Code BeamId,
                    const corsika::particles::Code TargetId,
                    const corsika::units::si::HEPEnergyType CoMenergy);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle&);

    /**
       In this function PYTHIA is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::process::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("pythia");
    Pythia8::Pythia fPythia;
    Pythia8::SigmaTotal fSigma;
    const bool fInternalDecays = true;
  };

} // namespace corsika::process::pythia

#endif
