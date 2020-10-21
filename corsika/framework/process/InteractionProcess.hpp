n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <type_traits>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>

namespace corsika {

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
    GrammageType GetInteractionLength(TParticle& p);

    template <typename TParticle>
    InverseGrammageType GetInverseInteractionLength(TParticle& p) {
      return 1. / GetRef().GetInteractionLength(p);
    }
  };

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const InteractionProcess<T>* impl);

} // namespace corsika
