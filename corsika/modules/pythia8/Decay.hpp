/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/process/DecayProcess.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/modules/pythia8/Pythia8.hpp>

namespace corsika::pythia8 {

  typedef corsika::Vector<hepmomentum_d> MomentumVector;

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
    TimeType GetLifetime(TParticle const&);

    template <typename TProjectile>
    void DoDecay(TProjectile&);

  private:
    Pythia8::Pythia fPythia;
  };

} // namespace corsika::pythia8

#include <corsika/detail/modules/pythia8/Decay.inl>
