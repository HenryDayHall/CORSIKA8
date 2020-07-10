/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Pythia8/Pythia.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/DecayProcess.h>

namespace corsika::process {

  namespace pythia {

    typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

    class Decay : public corsika::process::DecayProcess<Decay> {
      int fCount = 0;
      bool handleAllDecays_ = true;

    public:
      Decay();
      Decay(std::set<particles::Code>);
      ~Decay();

      // is Pythia::Decay set to handle the decay of this particle?
      bool IsDecayHandled(const corsika::particles::Code);

      // is decay possible in principle?
      bool CanHandleDecay(const corsika::particles::Code);

      // set Pythia::Decay to handle the decay of this particle!
      void SetHandleDecay(const corsika::particles::Code);
      // set Pythia::Decay to handle the decay of this list of particles!
      void SetHandleDecay(const std::vector<particles::Code>);
      // set Pythia::Decay to handle all particle decays
      void SetHandleAllDecays();

      // print internal configuration for this particle
      void PrintDecayConfig(const corsika::particles::Code);
      // print configuration of decays in corsika
      void PrintDecayConfig();

      bool CanDecay(const corsika::particles::Code);

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

      template <typename TProjectile>
      void DoDecay(TProjectile&);

    private:
      void SetUnstable(const corsika::particles::Code);
      void SetStable(const corsika::particles::Code);
      void SetStable(const std::vector<particles::Code>);
      bool IsStable(const corsika::particles::Code);

      Pythia8::Pythia fPythia;
      std::set<particles::Code> handledDecays_;
    };

  } // namespace pythia
} // namespace corsika::process
