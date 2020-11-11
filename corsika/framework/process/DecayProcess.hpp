n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  /**
     \class DecayProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type DecayProcess<T>

   */

  template <typename TDerived>
  struct DecayProcess : BaseProcess<TDerived> {
  public:
    using BaseProcess<TDerived>::ref;

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoDecay...
    template <typename TParticle>
    EProcessReturn doDecay(TParticle&);

    template <typename TParticle>
    TimeType getLifetime(TParticle const&);

    template <typename TParticle>
    InverseTimeType getInverseLifetime(TParticle const& particle) {
      return 1. / ref().getLifetime(particle);
    }

} // namespace corsika
