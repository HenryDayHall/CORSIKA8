/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Random.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/sequence/DecayProcess.hpp>

#include <set>
#include <vector>

namespace corsika::sibyll {

    class Decay : public corsika::DecayProcess<Decay> {
      int fCount = 0;
      bool handleAllDecays_ = true;

    public:
      Decay();
      Decay(std::set< Code>);
      ~Decay();

      void Init();

            void SetStable(const std::vector< Code>);
      void SetUnstable(const std::vector< Code>);

      void SetStable(const corsika::Code);
      void SetUnstable(const corsika::Code);

      // internally set all particles to decay/not to decay
      void SetAllUnstable();
      void SetAllStable();

      // will this particle be stable in sibyll ?
      bool IsStable(const corsika::Code);
      // will this particle decay in sibyll ?
      bool IsUnstable(const corsika::Code);
      // set particle with input code to decay or not
      void SetDecay(const  Code, const bool);

      
      void PrintDecayConfig(const corsika::Code);
      void PrintDecayConfig();
      void SetHadronsUnstable();

      // is Sibyll::Decay set to handle the decay of this particle?
      bool IsDecayHandled(const corsika::Code);

      // is decay possible in principle?
      bool CanHandleDecay(const corsika::Code);

      // set Sibyll::Decay to handle the decay of this particle!
      void SetHandleDecay(const corsika::Code);
      // set Sibyll::Decay to handle the decay of this list of particles!
      void SetHandleDecay(const std::vector< Code>);
      // set Sibyll::Decay to handle all particle decays
      void SetHandleAllDecay();

      template <typename TParticle>
      corsika::units::si::TimeType GetLifetime(TParticle const&) const;

      /**
       In this function SIBYLL is called to produce to decay the input particle.
     */

      template <typename TSecondaryParticle>
      void DoDecay(TSecondaryParticle&);

    private:
      // internal routines to set particles stable and unstable in the COMMON blocks in
      // sibyll

      std::set< Code> handledDecays_;
    };

} // namespace corsika::sibyll


#include <corsika/detail/modules/sibyll/Decay.inl>

