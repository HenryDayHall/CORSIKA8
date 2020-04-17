/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Pythia8/Pythia.h>

#include <iostream>
#include <vector>
#include <tuple>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/sequence/DecayProcess.hpp>

namespace corsika {

  namespace pythia {

    typedef corsika::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

    class Decay : public corsika::DecayProcess<Decay>
    {

    const std::vector<particles::Code> fTrackedParticles;
    int fCount = 0;

    public:

      Decay(std::vector<corsika::Code>);
      ~Decay();

      void SetParticleListStable(std::vector<particles::Code> const& );
      void SetUnstable(const corsika::Code);
      void SetStable(const corsika::Code);

      corsika::units::si::TimeType GetLifetime(corsika::Stack::ParticleType const&);

      void DoDecay(corsika::StackView::ParticleType&);

    private:
      void SetUnstable(const corsika::particles::Code);
      void SetStable(const corsika::particles::Code);
      void SetStable(const std::vector<particles::Code>);
      bool IsStable(const corsika::particles::Code);

      Pythia8::Pythia fPythia;
      std::set<particles::Code> handledDecays_;
    };

  } // namespace pythia
} // namespace corsika

#include <corsika/detail/process/Pythia/Decay.inl>
