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

#include <corsika/detail/framework/process/InteractionProcess.hpp> // for extra traits, method/interface checking

namespace corsika {

  /**
     @ingroup Processes
     @{

     Process describing the interaction of particles

     Create a new InteractionProcess, e.g. for XYModel, via
     @code
     class XYModel : public InteractionProcess<XYModel> {};
     @endcode

     and provide the two necessary interface methods
     @code
     template <typename TSecondaryView>
     void XYModel::doInteraction(TSecondaryView&);

     template <typename TParticle>
     GrammageType XYModel::getInteractionLength(TParticle const&)
     @endcode

     Where, of course, SecondaryView and Particle are the valid
     classes to access particles on the Stack. Those two methods do
     not need to be templated, they could use the types
     e.g. corsika::setup::Stack::particle_type -- but by the cost of
     loosing all flexibility otherwise provided.

     SecondaryView allows to retrieve the properties of the projectile
     particles, AND to store new particles (secondaries) which then
     subsequently can be processes by SecondariesProcess. This is how
     the output of interactions can be studied right away.

   */

  template <typename TDerived>
  class InteractionProcess : public BaseProcess<TDerived> {

  public:
    using BaseProcess<TDerived>::ref;

    template <typename TParticle>
    InverseGrammageType getInverseInteractionLength(TParticle const& particle) {

      // interface checking on TProcess1
      static_assert(
          has_method_getInteractionLength_v<TDerived, GrammageType, TParticle const&>,
          "TDerived has no method with correct signature \"GrammageType "
          "getInteractionLength(TParticle const&)\" required for "
          "InteractionProcess<TDerived>. ");

      return 1. / ref().getInteractionLength(particle);
    }
  };

  /**
   * ProcessTraits specialization to flag InteractionProcess objects
   **/
  template <typename TProcess>
  struct is_interaction_process<
      TProcess, std::enable_if_t<
                    std::is_base_of_v<InteractionProcess<typename std::decay_t<TProcess>>,
                                      typename std::decay_t<TProcess>>>>
      : std::true_type {};

  /** @} */

} // namespace corsika
