/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_process_pythia_decay_h_
#define _include_corsika_process_pythia_decay_h_

#include <Pythia8/Pythia.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/DecayProcess.h>

namespace corsika::process {

  namespace pythia {

    typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

    class Decay : public corsika::process::DecayProcess<Decay> {
      const std::vector<particles::Code> fTrackedParticles;
      int fCount = 0;

    public:
      Decay(std::vector<corsika::particles::Code>);
      ~Decay();
      void Init();

      void SetParticleListStable(const std::vector<particles::Code>);
      void SetUnstable(const corsika::particles::Code);
      void SetStable(const corsika::particles::Code);

      template <typename TParticle>
      corsika::units::si::TimeType GetLifetime(TParticle const&);

      template <typename TProjectile>
      void DoDecay(TProjectile&);

    private:
      Pythia8::Pythia fPythia;
    };

  } // namespace pythia
} // namespace corsika::process

#endif
