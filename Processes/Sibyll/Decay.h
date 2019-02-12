
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_process_sibyll_decay_h_
#define _include_corsika_process_sibyll_decay_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/DecayProcess.h>

namespace corsika::process {

  namespace sibyll {

    class Decay : public corsika::process::DecayProcess<Decay> {
      int fCount = 0;

    public:
      Decay();
      ~Decay();
      void Init();

      void setTrackedParticlesStable();
      void setUnstable(const corsika::particles::Code pCode);
      void setStable(const corsika::particles::Code pCode);
      void setAllStable();
      void setHadronsUnstable();

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle const& p);

      template <typename Particle, typename Stack>
      void DoDecay(Particle& p, Stack&);
    };
  } // namespace sibyll
} // namespace corsika::process

#endif
