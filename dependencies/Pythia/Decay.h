/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Pythia8/Pythia.h>
#include <corsika/framework/coreParticleProperties.h>
#include <corsika/process/DecayProcess.h>

namespace corsika {

  namespace pythia {

    typedef corsika::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

    class Decay : public corsika::DecayProcess<Decay> {
      int fCount = 0;
      bool handleAllDecays_ = true;

    public:
      Decay();
      Decay(std::set<Code>);
      ~Decay();

      // is Pythia::Decay set to handle the decay of this particle?
      bool IsDecayHandled(const corsika::Code);

      // is decay possible in principle?
      bool CanHandleDecay(const corsika::Code);

      // set Pythia::Decay to handle the decay of this particle!
      void SetHandleDecay(const corsika::Code);
      // set Pythia::Decay to handle the decay of this list of particles!
      void SetHandleDecay(const std::vector<Code>);
      // set Pythia::Decay to handle all particle decays
      void SetHandleAllDecays();

      // print internal configuration for this particle
      void PrintDecayConfig(const corsika::Code);
      // print configuration of decays in corsika
      void PrintDecayConfig();

      bool CanDecay(const corsika::Code);

      /**
       In this function PYTHIA is asked for the lifetime of the input particle.
       Unknown particles should return an infinite lifetime so that another decay process
       can act on the particle later on.
     */

      template <typename TParticle>
      corsika::units::si::TimeType GetLifetime(TParticle const&);

      /**
       In this function PYTHIA is called to execute the decay of the input particle.
     */

      template <typename TSecondaryView>
      void DoDecay(TSecondaryView&);

    private:
      void SetUnstable(const corsika::Code);
      void SetStable(const corsika::Code);
      void SetStable(const std::vector<Code>);
      bool IsStable(const corsika::Code);

      Pythia8::Pythia fPythia;
      std::set<Code> handledDecays_;
    };

  } // namespace pythia
} // namespace corsika
