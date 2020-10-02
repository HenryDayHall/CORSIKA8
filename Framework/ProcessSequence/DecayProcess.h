/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/process/ProcessReturn.h> // for convenience
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

  /**
     \class DecayProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type DecayProcess<T>

   */

  template <typename TDerived>
  struct DecayProcess : BaseProcess<TDerived> {

    using BaseProcess<TDerived>::GetRef;

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoDecay...
    template <typename TParticle>
    EProcessReturn DoDecay(TParticle&);

    template <typename TParticle>
    corsika::units::si::TimeType GetLifetime(TParticle& p);

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetInverseLifetime(TParticle& vP) {
      return 1. / GetRef().GetLifetime(vP);
    }
  };

} // namespace corsika::process
