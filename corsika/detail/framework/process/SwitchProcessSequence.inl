/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/process/ProcessTraits.hpp>
#include <corsika/framework/process/BoundaryCrossingProcess.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/framework/process/ContinuousProcessStepLength.hpp>
#include <corsika/framework/process/ContinuousProcessIndex.hpp>
#include <corsika/framework/process/DecayProcess.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/framework/process/StackProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <cmath>
#include <limits>
#include <type_traits>

namespace corsika {

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle>
  inline ProcessReturn SwitchProcessSequence<
      TCondition, TSequence, USequence, IndexStart, IndexProcess1,
      IndexProcess2>::doBoundaryCrossing(TParticle& particle,
                                         typename TParticle::node_type const& from,
                                         typename TParticle::node_type const& to) {
    if (select_(particle)) {
      if constexpr (is_boundary_process_v<process1_type> ||
                    process1_type::is_process_sequence) {

        // interface checking on TSequence
        if constexpr (is_boundary_process_v<process1_type>) {

          static_assert(
              has_method_doBoundaryCrossing_v<TSequence, ProcessReturn, TParticle&>,
              "TDerived has no method with correct signature \"ProcessReturn "
              "doBoundaryCrossing(TParticle&, VolumeNode const&, VolumeNode const&)\" "
              "required for "
              "BoundaryCrossingProcess<TDerived>. ");
        }

        return A_.doBoundaryCrossing(particle, from, to);
      }
    } else {

      if constexpr (is_boundary_process_v<process2_type> ||
                    process2_type::is_process_sequence) {

        // interface checking on USequence
        if constexpr (is_boundary_process_v<process2_type>) {

          static_assert(
              has_method_doBoundaryCrossing_v<USequence, ProcessReturn, TParticle>,
              "TDerived has no method with correct signature \"ProcessReturn "
              "doBoundaryCrossing(TParticle&, VolumeNode const&, VolumeNode const&)\" "
              "required for "
              "BoundaryCrossingProcess<TDerived>. ");
        }

        return B_.doBoundaryCrossing(particle, from, to);
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ProcessReturn SwitchProcessSequence<
      TCondition, TSequence, USequence, IndexStart, IndexProcess1,
      IndexProcess2>::doContinuous(TParticle& particle, TTrack& vT,
                                   ContinuousProcessIndex const idLimit) {
    if (select_(particle)) {
      if constexpr (process1_type::is_process_sequence) {
        return A_.doContinuous(particle, vT, idLimit);
      }
      if constexpr (is_continuous_process_v<process1_type>) {

        static_assert(
            has_method_doContinuous_v<TSequence, ProcessReturn, TParticle&, TTrack&> ||
                has_method_doContinuous_v<TSequence, ProcessReturn, TParticle&,
                                          TTrack const&> ||
                has_method_doContinuous_v<TSequence, ProcessReturn, TParticle const&,
                                          TTrack const&>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doContinuous(TParticle[const]&,TTrack[const]&,bool)\" required for "
            "ContinuousProcess<TDerived>. ");

        return A_.doContinuous(particle, vT,
                               idLimit == ContinuousProcessIndex(IndexProcess1));
      }
    } else {
      if constexpr (process2_type::is_process_sequence) {
        return B_.doContinuous(particle, vT, idLimit);
      }
      if constexpr (is_continuous_process_v<process2_type>) {

        // interface checking on USequence
        static_assert(
            has_method_doContinuous_v<USequence, ProcessReturn, TParticle&, TTrack&> ||
                has_method_doContinuous_v<USequence, ProcessReturn, TParticle&,
                                          TTrack const&> ||
                has_method_doContinuous_v<USequence, ProcessReturn, TParticle const&,
                                          TTrack const&>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doContinuous(TParticle [const]&,TTrack[const]&,bool)\" required for "
            "ContinuousProcess<TDerived>. ");

        return B_.doContinuous(particle, vT,
                               idLimit == ContinuousProcessIndex(IndexProcess2));
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TSecondaries>
  inline void
  SwitchProcessSequence<TCondition, TSequence, USequence, IndexStart, IndexProcess1,
                        IndexProcess2>::doSecondaries(TSecondaries& vS) {
    const auto& particle = vS.parent();
    if (select_(particle)) {
      if constexpr (is_secondaries_process_v<process1_type> ||
                    process1_type::is_process_sequence) {

        // interface checking on TSequence
        static_assert(
            has_method_doSecondaries_v<TSequence, void, TSecondaries&> ||
                has_method_doSecondaries_v<TSequence, void, TSecondaries const&>,
            "TDerived has no method with correct signature \"void "
            "doSecondaries(TStackView [const]&)\" required for "
            "SecondariesProcessProcess<TDerived>. ");

        A_.doSecondaries(vS);
      }
    } else {
      if constexpr (is_secondaries_process_v<process2_type> ||
                    process2_type::is_process_sequence) {

        // interface checking on USequence
        static_assert(
            has_method_doSecondaries_v<USequence, void, TSecondaries&> ||
                has_method_doSecondaries_v<USequence, void, TSecondaries const&>,
            "TDerived has no method with correct signature \"void "
            "doSecondaries(TStackView [const]&)\" required for "
            "SecondariesProcessProcess<TDerived>. ");

        B_.doSecondaries(vS);
      }
    }
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ContinuousProcessStepLength
  SwitchProcessSequence<TCondition, TSequence, USequence, IndexStart, IndexProcess1,
                        IndexProcess2>::getMaxStepLength(TParticle& particle,
                                                         TTrack& vTrack) {
    if (select_(particle)) {
      if constexpr (process1_type::is_process_sequence) {
        return A_.getMaxStepLength(particle, vTrack);
      }
      if constexpr (is_continuous_process_v<process1_type>) {

        // interface checking on TSequence
        static_assert(has_method_getMaxStepLength_v<TSequence, LengthType,
                                                    TParticle const&, TTrack const&>,
                      "TDerived has no method with correct signature \"LengthType "
                      "getMaxStepLength(TParticle const&, TTrack const&)\" required for "
                      "ContinuousProcess<TDerived>. ");

        return ContinuousProcessStepLength(A_.getMaxStepLength(particle, vTrack),
                                           ContinuousProcessIndex(IndexProcess1));
      }
    } else {
      if constexpr (process2_type::is_process_sequence) {
        return B_.getMaxStepLength(particle, vTrack);
      }
      if constexpr (is_continuous_process_v<process2_type>) {

        // interface checking on USequence
        static_assert(has_method_getMaxStepLength_v<USequence, LengthType,
                                                    TParticle const&, TTrack const&>,
                      "TDerived has no method with correct signature \"LengthType "
                      "getMaxStepLength(TParticle const&, TTrack const&)\" required for "
                      "ContinuousProcess<TDerived>. ");

        return ContinuousProcessStepLength(B_.getMaxStepLength(particle, vTrack),
                                           ContinuousProcessIndex(IndexProcess2));
      }
    }

    // if no other process in the sequence implements it
    return ContinuousProcessStepLength(std::numeric_limits<double>::infinity() * meter);
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle>
  inline InverseGrammageType SwitchProcessSequence<
      TCondition, TSequence, USequence, IndexStart, IndexProcess1,
      IndexProcess2>::getInverseInteractionLength(TParticle&& particle) {

    if (select_(particle)) {
      if constexpr (is_interaction_process_v<process1_type> ||
                    process1_type::is_process_sequence) {
        return A_.getInverseInteractionLength(particle);
      }

    } else {

      if constexpr (is_interaction_process_v<process2_type> ||
                    process2_type::is_process_sequence) {
        return B_.getInverseInteractionLength(particle);
      }
    }
    return 0 * meter * meter / gram; // default value
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TSecondaryView>
  inline ProcessReturn SwitchProcessSequence<TCondition, TSequence, USequence, IndexStart,
                                             IndexProcess1, IndexProcess2>::
      selectInteraction(TSecondaryView& view,
                        [[maybe_unused]] InverseGrammageType lambda_inv_select,
                        [[maybe_unused]] InverseGrammageType lambda_inv_sum) {
    if (select_(view.parent())) {
      if constexpr (process1_type::is_process_sequence) {
        // if A_ is a process sequence --> check inside
        ProcessReturn const ret =
            A_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
        // if A_ did succeed, stop routine. Not checking other static branch B_.
        if (ret != ProcessReturn::Ok) { return ret; }
      } else if constexpr (is_interaction_process_v<process1_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += A_.getInverseInteractionLength(view.parent());
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select < lambda_inv_sum) {

          // interface checking on TSequence
          static_assert(has_method_doInteract_v<TSequence, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doInteraction(TSecondaryView&)\" required for "
                        "InteractionProcess<TDerived>. ");

          A_.doInteraction(view);
          return ProcessReturn::Interacted;
        }
      } // end branch A_

    } else {

      if constexpr (process2_type::is_process_sequence) {
        // if B_ is a process sequence --> check inside
        return B_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
      } else if constexpr (is_interaction_process_v<process2_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += B_.getInverseInteractionLength(view.parent());
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select < lambda_inv_sum) {

          // interface checking on TSequence
          static_assert(has_method_doInteract_v<USequence, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doInteraction(TSecondaryView&)\" required for "
                        "InteractionProcess<TDerived>. ");

          B_.doInteraction(view);
          return ProcessReturn::Interacted;
        }
      } // end branch B_
    }
    return ProcessReturn::Ok;
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle>
  inline InverseTimeType
  SwitchProcessSequence<TCondition, TSequence, USequence, IndexStart, IndexProcess1,
                        IndexProcess2>::getInverseLifetime(TParticle&& particle) {

    if (select_(particle)) {
      if constexpr (is_decay_process_v<process1_type> ||
                    process1_type::is_process_sequence) {
        return A_.getInverseLifetime(particle);
      }

    } else {

      if constexpr (is_decay_process_v<process2_type> ||
                    process2_type::is_process_sequence) {
        return B_.getInverseLifetime(particle);
      }
    }
    return 0 / second; // default value
  }

  template <typename TCondition, typename TSequence, typename USequence, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  // select decay process
  template <typename TSecondaryView>
  inline ProcessReturn SwitchProcessSequence<
      TCondition, TSequence, USequence, IndexStart, IndexProcess1,
      IndexProcess2>::selectDecay(TSecondaryView& view,
                                  [[maybe_unused]] InverseTimeType decay_inv_select,
                                  [[maybe_unused]] InverseTimeType decay_inv_sum) {
    if (select_(view.parent())) {
      if constexpr (process1_type::is_process_sequence) {
        // if A_ is a process sequence --> check inside
        ProcessReturn const ret = A_.selectDecay(view, decay_inv_select, decay_inv_sum);
        // if A_ did succeed, stop routine here (not checking other static branch B_)
        if (ret != ProcessReturn::Ok) { return ret; }
      } else if constexpr (is_decay_process_v<process1_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += A_.getInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select < decay_inv_sum) {
          // more pedagogical: rndm_select < decay_inv_sum / decay_inv_tot

          // interface checking on TSequence
          static_assert(has_method_doDecay_v<TSequence, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doDecay(TSecondaryView&)\" required for "
                        "DecayProcess<TDerived>. ");

          A_.doDecay(view);
          return ProcessReturn::Decayed;
        }
      } // end branch A_

    } else {

      if constexpr (process2_type::is_process_sequence) {
        // if B_ is a process sequence --> check inside
        return B_.selectDecay(view, decay_inv_select, decay_inv_sum);
      } else if constexpr (is_decay_process_v<process2_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += B_.getInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select < decay_inv_sum) {

          // interface checking on TSequence
          static_assert(has_method_doDecay_v<USequence, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doDecay(TSecondaryView&)\" required for "
                        "DecayProcess<TDerived>. ");

          B_.doDecay(view);
          return ProcessReturn::Decayed;
        }
      } // end branch B_
    }
    return ProcessReturn::Ok;
  }

} // namespace corsika
