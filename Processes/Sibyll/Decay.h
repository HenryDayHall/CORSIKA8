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
#include <corsika/process/SecondariesProcess.h>

#include <vector>

namespace corsika::process {

  namespace sibyll {

    class Decay : public corsika::process::DecayProcess<Decay> {
      int fCount = 0;

    public:
      Decay();
      ~Decay();
      void Init();

      void SetStable(const std::vector<particles::Code>);
      void SetUnstable(const std::vector<particles::Code>);
      void SetUnstable(const corsika::particles::Code);
      void SetStable(const corsika::particles::Code);
      void SetAllUnstable();
      void SetAllStable();
      bool IsStable(const corsika::particles::Code);
      bool IsUnstable(const corsika::particles::Code);
      void SetDecay(const particles::Code, const bool);

      void PrintDecayConfig(const corsika::particles::Code);
      void SetHadronsUnstable();

      template <typename TParticle>
      corsika::units::si::TimeType GetLifetime(TParticle const&) const;

      /**
       In this function SIBYLL is called to produce to decay the input particle.
     */

      template <typename TProjectile>
      void DoDecay(TProjectile&);

      template <typename TParticleView>
      EProcessReturn DoSecondaries(TParticleView&);
    };

  } // namespace sibyll

} // namespace corsika::process

#endif
