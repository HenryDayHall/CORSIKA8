/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_interactionprocess_h_
#define _include_corsika_interactionprocess_h_

#include <corsika/process/ProcessReturn.h> // for convenience
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

  /**
     \class InteractionProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type InteractionProcess<T>

   */

  template <typename derived>
  struct InteractionProcess {

    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }

    /// here starts the interface-definition part
    // -> enforce derived to implement DoInteraction...
    template <typename Particle>
    EProcessReturn DoInteraction(Particle&);

    template <typename TParticle>
    corsika::units::si::GrammageType GetInteractionLength(TParticle& p);

    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(TParticle& p) {
      return 1. / GetRef().GetInteractionLength(p);
    }
  };

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const InteractionProcess<T>* impl);

} // namespace corsika::process

#endif
