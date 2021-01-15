/*
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
     Process describing the interaction of particles

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type InteractionProcess<T>

   */

  template <typename TDerived>
  class InteractionProcess : public BaseProcess<TDerived> {
  public:
    using BaseProcess<TDerived>::ref;

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoInteraction...
    template <typename TParticle>
    void doInteraction(TParticle&);

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&);

    template <typename TParticle>
    InverseGrammageType getInverseInteractionLength(TParticle const& particle) {
      return 1. / ref().getInteractionLength(particle);
    }
  };

} // namespace corsika
