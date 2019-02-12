
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_sibyll_nuclearinteraction_h_
#define _corsika_process_sibyll_nuclearinteraction_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>

namespace corsika::environment {
  class Environment;
}

namespace corsika::process::sibyll {

  class Interaction; // fwd-decl

  /**
   *
   *
   **/

  class NuclearInteraction
      : public corsika::process::InteractionProcess<NuclearInteraction> {

    int fCount = 0;
    int fNucCount = 0;

  public:
    NuclearInteraction(corsika::environment::Environment const& env,
                       corsika::process::sibyll::Interaction& hadint);
    ~NuclearInteraction();
    void Init();

    template <typename Particle>
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(Particle& p, const corsika::particles::Code TargetId);

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle& p, Track&);

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack& s);

  private:
    corsika::environment::Environment const& fEnvironment;
    corsika::process::sibyll::Interaction& fHadronicInteraction;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  };

} // namespace corsika::process::sibyll

#endif
