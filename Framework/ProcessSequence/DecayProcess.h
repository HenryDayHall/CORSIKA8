/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_decayprocess_h_
#define _include_corsika_decayprocess_h_

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

  template <typename derived>
  struct DecayProcess {
    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }

    /// here starts the interface-definition part
    // -> enforce derived to implement DoDecay...
    template <typename Particle>
    EProcessReturn DoDecay(Particle&);

    template <typename Particle>
    corsika::units::si::TimeType GetLifetime(Particle& p);

    template <typename Particle>
    corsika::units::si::InverseTimeType GetInverseLifetime(Particle& vP) {
      return 1. / GetRef().GetLifetime(vP);
    }
  };

  // overwrite the default trait class, to mark DecayProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const DecayProcess<T>* impl);

} // namespace corsika::process

#endif
