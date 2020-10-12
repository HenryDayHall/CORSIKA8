/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

  /**
     \class InteractionProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type InteractionProcess<T>

   */

  template <typename TDerived>
  class InteractionProcess : public BaseProcess<TDerived> {
  public:
    using BaseProcess<TDerived>::GetRef;

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoInteraction...
    template <typename TParticle>
    EProcessReturn DoInteraction(TParticle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(const TParticle&);

    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(
        const TParticle& particle) {
      return 1. / GetRef().GetInteractionLength(particle);
    }

    /*
    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(TParticle&&
    particle) { auto p = std::move(particle); return 1. /
    GetRef().GetInteractionLength(p);
      }*/
  };

} // namespace corsika::process
