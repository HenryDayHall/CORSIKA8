
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
      std::vector<particles::Code> fTrackedParticles;
      int fCount = 0;

    public:
      Decay(std::vector<particles::Code>);
      ~Decay();
      void Init();

      void SetParticleListStable(const std::vector<particles::Code>);
      void SetUnstable(const corsika::particles::Code);
      void SetStable(const corsika::particles::Code);
      void SetAllStable();
      void SetHadronsUnstable();

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle const&);

      template <typename Projectile>
      void DoDecay(Projectile&);
    };
  } // namespace sibyll
} // namespace corsika::process

#endif
