/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <type_traits>

#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include "corsika/framework/sequence/ProcessReturn.hpp" // for convenience

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
    using BaseProcess<TDerived>::GetRef;

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoDecay...
    template <typename TParticle>
    EProcessReturn DoDecay(TParticle&);

    template <typename TParticle>
    corsika::units::si::TimeType GetLifetime(const TParticle&);

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetInverseLifetime(const TParticle& particle) {
      return 1. / GetRef().GetLifetime(particle);
    }

    /*    template <typename TParticle>
    corsika::units::si::InverseTimeType GetInverseInteractionLength(TParticle&& particle)
    { auto p = std::move(particle); return 1. / GetRef().GetLifetime(p);
      }*/
  };

} // namespace corsika
