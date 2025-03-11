/*
 * (c) Copyright 2024 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

namespace corsika::setup {

  /**
    \file SetupC7TrackedParticles.hpp

    set of particles that should be tracked by corsika. all others should decay.
    this is the corsika 7 configuration
   */

  std::set<Code> C7trackedParticles{
      Code::Proton,        Code::Neutron,   Code::AntiProton,   Code::AntiNeutron,
      Code::PiPlus,        Code::PiMinus,   Code::Pi0,          Code::KPlus,
      Code::KMinus,        Code::K0Long,    Code::K0Short,      Code::Lambda,
      Code::LambdaBar,     Code::SigmaPlus, Code::SigmaPlusBar, Code::SigmaMinus,
      Code::SigmaMinusBar, Code::Xi0,       Code::Xi0Bar,       Code::OmegaMinus,
      Code::OmegaPlusBar,  Code::MuPlus,    Code::MuMinus};
} // namespace corsika::setup