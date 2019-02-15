
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

namespace corsika::environment {
  class Environment;
}

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int fCount = 0;
    int fNucCount = 0;
    bool fInitialized = false;

  public:
    Interaction(corsika::environment::Environment const& env);
    ~Interaction();

    void Init();

    bool WasInitialized() { return fInitialized; }
    bool ValidCoMEnergy(corsika::units::si::HEPEnergyType ecm) {
      using namespace corsika::units::si;
      return (10_GeV < ecm) && (ecm < 1_PeV);
    }

    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(const corsika::particles::Code BeamId,
                    const corsika::particles::Code TargetId,
                    const corsika::units::si::HEPEnergyType CoMenergy);

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle&, Track&);

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle&, Stack&);

  private:
    corsika::environment::Environment const& fEnvironment;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  };

} // namespace corsika::process::sibyll

#endif
