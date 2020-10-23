/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/SecondariesProcess.h>

#include <set>
#include <vector>

namespace corsika::process {

  namespace sibyll {

    class Decay : public corsika::process::DecayProcess<Decay> {
      int count_ = 0;
      bool handleAllDecays_ = true;
      bool sibyll_listing_ = false;

    public:
      Decay(const bool sibyll_listing = false);
      Decay(std::set<particles::Code>);
      ~Decay();

      void PrintDecayConfig(const corsika::particles::Code);
      void PrintDecayConfig();
      void SetHadronsUnstable();

      // is Sibyll::Decay set to handle the decay of this particle?
      bool IsDecayHandled(const corsika::particles::Code);

      // is decay possible in principle?
      bool CanHandleDecay(const corsika::particles::Code);

      // set Sibyll::Decay to handle the decay of this particle!
      void SetHandleDecay(const corsika::particles::Code);
      // set Sibyll::Decay to handle the decay of this list of particles!
      void SetHandleDecay(const std::vector<particles::Code>);
      // set Sibyll::Decay to handle all particle decays
      void SetHandleAllDecay();

      template <typename TParticle>
      corsika::units::si::TimeType GetLifetime(TParticle const&);

      /**
       In this function SIBYLL is called to produce to decay the input particle.
     */

      template <typename TSecondaryView>
      void DoDecay(TSecondaryView&);

      template <typename TParticleView>
      EProcessReturn DoSecondaries(TParticleView&);

    private:
      // internal routines to set particles stable and unstable in the COMMON blocks in
      // sibyll
      void SetStable(const std::vector<particles::Code>);
      void SetUnstable(const std::vector<particles::Code>);

      void SetStable(const corsika::particles::Code);
      void SetUnstable(const corsika::particles::Code);

      // internally set all particles to decay/not to decay
      void SetAllUnstable();
      void SetAllStable();

      // will this particle be stable in sibyll ?
      bool IsStable(const corsika::particles::Code);
      // will this particle decay in sibyll ?
      bool IsUnstable(const corsika::particles::Code);
      // set particle with input code to decay or not
      void SetDecay(const particles::Code, const bool);

      std::set<particles::Code> handledDecays_;
    };

  } // namespace sibyll

} // namespace corsika::process
