
/**
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
    template <typename Particle, typename Stack>
    inline EProcessReturn DoDecay(Particle&, Stack&) const;

    template <typename Particle>
    inline double GetLifetime(Particle& p) const;

    template <typename Particle>
    inline double GetInverseLifetime(Particle& p) const {
      return 1. / GetRef().GetLifetime(p);
    }
  };

} // namespace corsika::process

#endif
