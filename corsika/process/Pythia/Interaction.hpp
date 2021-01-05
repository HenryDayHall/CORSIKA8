/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Pythia8/Pythia.h>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <tuple>

namespace corsika::pythia {

  class Interaction : public corsika::InteractionProcess<Interaction> {

    int fCount = 0;
    bool fInitialized = false;
    bool print_listing_ = false;

  public:
    Interaction(const bool print_listing = false);
    ~Interaction() = default;

    void SetParticleListStable(std::vector<particles::Code> const&);
    void SetUnstable(const corsika::Code);
    void SetStable(const corsika::Code);

    bool WasInitialized() { return fInitialized; }
    bool ValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) {
      using namespace corsika::units::si;
      return (10_GeV < ecm) && (ecm < 1_PeV);
    }

    bool CanInteract(const corsika::Code);
    void ConfigureLabFrameCollision(const corsika::Code, const corsika::Code,
                                    const corsika::units::si::HEPEnergyType);
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::Code BeamId, const corsika::Code TargetId,
                    const corsika::units::si::HEPEnergyType CoMenergy);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(const TParticle&);

    /**
       In this function PYTHIA is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TProjectile>
    corsika::EProcessReturn DoInteraction(TProjectile&);

  private:
    corsika::RNG& fRNG = corsika::RNGManager::GetInstance().GetRandomStream("pythia");
    Pythia8::Pythia fPythia;
    Pythia8::SigmaTotal fSigma;
    const bool fInternalDecays = true;
  };

} // namespace corsika::pythia

#endif
