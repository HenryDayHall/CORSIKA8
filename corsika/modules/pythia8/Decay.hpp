/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/sequence/DecayProcess.hpp>

#include <corsika/modules/pythia8/Pythia8.hpp>

namespace corsika::pythia8 {

  typedef corsika::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  class Decay : public corsika::DecayProcess<Decay> {
    const std::vector<corsika::Code> fTrackedParticles;
    int fCount = 0;

  public:
    Decay(std::vector<corsika::Code>);
    ~Decay();
    void Init();

    void SetParticleListStable(const std::vector<corsika::Code>);
    void SetUnstable(const corsika::Code);
    void SetStable(const corsika::Code);

    template <typename TParticle>
    corsika::units::si::TimeType GetLifetime(TParticle const&);

    template <typename TProjectile>
    void DoDecay(TProjectile&);

  private:
    Pythia8::Pythia fPythia;
  };

} // namespace corsika::pythia8

#include <corsika/detail/modules/pythia8/Decay.inl>
