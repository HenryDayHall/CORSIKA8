/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/process/BoundaryCrossingProcess.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/framework/process/ContinuousProcessStepLength.hpp>
#include <corsika/framework/process/ContinuousProcessIndex.hpp>
#include <corsika/framework/process/DecayProcess.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/framework/process/StackProcess.hpp>

#include <cmath>
#include <limits>
#include <type_traits>

namespace corsika {

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  inline ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                         IndexProcess2>::ProcessSequence(TProcess1 in_A, TProcess2 in_B)
      : A_(in_A)
      , B_(in_B) {

    // make sure only BaseProcess types TProcess1/2 are passed

    static_assert(is_process_v<TProcess1>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 1");
    static_assert(is_process_v<TProcess2>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 2");
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TParticle>
  inline ProcessReturn ProcessSequence<
      TProcess1, TProcess2, IndexStart, IndexProcess1,
      IndexProcess2>::doBoundaryCrossing(TParticle& particle,
                                         typename TParticle::node_type const& from,
                                         typename TParticle::node_type const& to) {

    ProcessReturn ret = ProcessReturn::Ok;

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_boundary_process_v<process1_type> ||
                    process1_type::is_process_sequence) {

        // interface checking on TProcess1
        static_assert(
            has_method_doBoundaryCrossing_v<TProcess1, ProcessReturn, TParticle>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doBoundaryCrossing(TParticle&, VolumeNode const&, VolumeNode const&)\" "
            "required for "
            "BoundaryCrossingProcess<TDerived>. ");

        ret |= A_.doBoundaryCrossing(particle, from, to);
      }
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_boundary_process_v<process2_type> ||
                    process2_type::is_process_sequence) {

        // interface checking on TProcess2
        static_assert(
            has_method_doBoundaryCrossing_v<TProcess2, ProcessReturn, TParticle>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doBoundaryCrossing(TParticle&, VolumeNode const&, VolumeNode const&)\" "
            "required for "
            "BoundaryCrossingProcess<TDerived>. ");

        ret |= B_.doBoundaryCrossing(particle, from, to);
      }
    }

    return ret;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ProcessReturn
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1, IndexProcess2>::
      doContinuous(TParticle& particle, TTrack& vT,
                   [[maybe_unused]] ContinuousProcessIndex const limitId) {
    ProcessReturn ret = ProcessReturn::Ok;

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (process1_type::is_process_sequence) {

        ret |= A_.doContinuous(particle, vT, limitId);
      } else if constexpr (is_continuous_process_v<process1_type>) {

        // interface checking on TProcess1
        static_assert(
            has_method_doContinuous_v<TProcess1, ProcessReturn, TParticle&, TTrack&> ||
                has_method_doContinuous_v<TProcess1, ProcessReturn, TParticle&,
                                          TTrack const&> ||
                has_method_doContinuous_v<TProcess1, ProcessReturn, TParticle const&,
                                          TTrack const&>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doContinuous(TParticle [const]&,TTrack [const]&,bool)\" required for "
            "ContinuousProcess<TDerived>. ");

        ret |= A_.doContinuous(particle, vT,
                               limitId == ContinuousProcessIndex(IndexProcess1));
      }
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (process2_type::is_process_sequence) {
        ret |= B_.doContinuous(particle, vT, limitId);
      } else if constexpr (is_continuous_process_v<process2_type>) {

        // interface checking on TProcess2
        static_assert(
            has_method_doContinuous_v<TProcess2, ProcessReturn, TParticle&, TTrack&> ||
                has_method_doContinuous_v<TProcess2, ProcessReturn, TParticle&,
                                          TTrack const&> ||
                has_method_doContinuous_v<TProcess2, ProcessReturn, TParticle const&,
                                          TTrack const&>,
            "TDerived has no method with correct signature \"ProcessReturn "
            "doContinuous(TParticle [const]&,TTrack [const]&,bool)\" required for "
            "ContinuousProcess<TDerived>. ");

        ret |= B_.doContinuous(particle, vT,
                               limitId == ContinuousProcessIndex(IndexProcess2));
      }
    }

    return ret;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TSecondaries>
  inline void
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                  IndexProcess2>::doSecondaries([[maybe_unused]] TSecondaries& vS) {

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_secondaries_process_v<process1_type> ||
                    process1_type::is_process_sequence) {

        // interface checking on TProcess1
        static_assert(
            has_method_doSecondaries_v<TProcess1, void, TSecondaries&> ||
                has_method_doSecondaries_v<TProcess1, void, TSecondaries const&>,
            "TDerived has no method with correct signature \"void "
            "doSecondaries(TStackView [const]&)\" required for "
            "SecondariesProcessProcess<TDerived>. ");

        A_.doSecondaries(vS);
      }
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_secondaries_process_v<process2_type> ||
                    process2_type::is_process_sequence) {

        // interface checking on TProcess2
        static_assert(
            has_method_doSecondaries_v<TProcess2, void, TSecondaries&> ||
                has_method_doSecondaries_v<TProcess2, void, TSecondaries const&>,
            "TDerived has no method with correct signature \"void "
            "doSecondaries(TStackView [const]&)\" required for "
            "SecondariesProcessProcess<TDerived>. ");

        B_.doSecondaries(vS);
      }
    }
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  inline bool ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                              IndexProcess2>::checkStep() {
    bool ret = false;
    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_stack_process_v<process1_type> ||
                    (process1_type::is_process_sequence &&
                     !process1_type::is_switch_process_sequence)) {
        ret |= A_.checkStep();
      }
    }
    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_stack_process_v<process2_type> ||
                    (process2_type::is_process_sequence &&
                     !process2_type::is_switch_process_sequence)) {
        ret |= B_.checkStep();
      }
    }
    return ret;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TStack>
  inline void ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                              IndexProcess2>::doStack(TStack& stack) {
    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_stack_process_v<process1_type> ||
                    (process1_type::is_process_sequence &&
                     !process1_type::is_switch_process_sequence)) {

        // interface checking on TProcess1
        static_assert(has_method_doStack_v<TProcess1, void, TStack&> ||
                          has_method_doStack_v<TProcess1, void, TStack const&>,
                      "TDerived has no method with correct signature \"void "
                      "doStack(TStack [const]&)\" required for "
                      "StackProcess<TDerived>. ");

        if (A_.checkStep()) { A_.doStack(stack); }
      }
    }
    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_stack_process_v<process2_type> ||
                    (process2_type::is_process_sequence &&
                     !process2_type::is_switch_process_sequence)) {

        // interface checking on TProcess1
        static_assert(has_method_doStack_v<TProcess2, void, TStack&> ||
                          has_method_doStack_v<TProcess2, void, TStack const&>,
                      "TDerived has no method with correct signature \"void "
                      "doStack(TStack [const]&)\" required for "
                      "StackProcess<TDerived>. ");

        if (B_.checkStep()) { B_.doStack(stack); }
      }
    }
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ContinuousProcessStepLength
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                  IndexProcess2>::getMaxStepLength(TParticle& particle, TTrack& vTrack) {
    // if no other process in the sequence implements it
    ContinuousProcessStepLength max_length(std::numeric_limits<double>::infinity() *
                                           meter);

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (process1_type::is_process_sequence) {
        ContinuousProcessStepLength const step = A_.getMaxStepLength(particle, vTrack);
        max_length = std::min(max_length, step);
      } else if constexpr (is_continuous_process_v<process1_type>) {

        // interface checking on TProcess1
        static_assert(has_method_getMaxStepLength_v<TProcess1, LengthType,
                                                    TParticle const&, TTrack const&>,
                      "TDerived has no method with correct signature \"LengthType "
                      "getMaxStepLength(TParticle const&, TTrack const&)\" required for "
                      "ContinuousProcess<TDerived>. ");

        ContinuousProcessStepLength const step(A_.getMaxStepLength(particle, vTrack),
                                               ContinuousProcessIndex(IndexProcess1));
        max_length = std::min(max_length, step);
      }
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (process2_type::is_process_sequence) {
        ContinuousProcessStepLength const step = B_.getMaxStepLength(particle, vTrack);
        max_length = std::min(max_length, step);
      } else if constexpr (is_continuous_process_v<process2_type>) {

        // interface checking on TProcess2
        static_assert(has_method_getMaxStepLength_v<TProcess2, LengthType,
                                                    TParticle const&, TTrack const&>,
                      "TDerived has no method with correct signature \"LengthType "
                      "getMaxStepLength(TParticle const&, TTrack const&)\" required for "
                      "ContinuousProcess<TDerived>. ");

        ContinuousProcessStepLength const step(B_.getMaxStepLength(particle, vTrack),
                                               ContinuousProcessIndex(IndexProcess2));
        max_length = std::min(max_length, step);
      }
    }
    return max_length;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TParticle>
  inline InverseGrammageType
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                  IndexProcess2>::getInverseInteractionLength(TParticle&& particle) {

    InverseGrammageType tot = 0 * meter * meter / gram; // default value

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_interaction_process_v<process1_type> ||
                    process1_type::is_process_sequence) {
        tot += A_.getInverseInteractionLength(particle);
      }
    }
    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_interaction_process_v<process2_type> ||
                    process2_type::is_process_sequence) {
        tot += B_.getInverseInteractionLength(particle);
      }
    }
    return tot;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TSecondaryView>
  inline ProcessReturn
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1, IndexProcess2>::
      selectInteraction(TSecondaryView& view,
                        [[maybe_unused]] InverseGrammageType lambda_inv_select,
                        [[maybe_unused]] InverseGrammageType lambda_inv_sum) {

    // TODO: add check for lambda_inv_select > lambda_inv_tot

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (process1_type::is_process_sequence) {
        // if A is a process sequence --> check inside
        ProcessReturn const ret =
            A_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
        // if A_ did succeed, stop routine. Not checking other static branch B_.
        if (ret != ProcessReturn::Ok) { return ret; }
      } else if constexpr (is_interaction_process_v<process1_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += A_.getInverseInteractionLength(view.parent());
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select <= lambda_inv_sum) {

          // interface checking on TProcess1
          static_assert(has_method_doInteract_v<TProcess1, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doInteraction(TSecondaryView&)\" required for "
                        "InteractionProcess<TDerived>. ");

          A_.template doInteraction(view);
          return ProcessReturn::Interacted;
        }
      } // end branch A
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid

      if constexpr (process2_type::is_process_sequence) {
        // if B_ is a process sequence --> check inside
        return B_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
      } else if constexpr (is_interaction_process_v<process2_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += B_.getInverseInteractionLength(view.parent());
        // soon as SecondaryView::parent() is migrated!
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select <= lambda_inv_sum) {

          // interface checking on TProcess1
          static_assert(has_method_doInteract_v<TProcess2, void, TSecondaryView&>,
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

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  template <typename TParticle>
  inline InverseTimeType
  ProcessSequence<TProcess1, TProcess2, IndexStart, IndexProcess1,
                  IndexProcess2>::getInverseLifetime(TParticle&& particle) {

    InverseTimeType tot = 0 / second; // default value

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (is_decay_process_v<process1_type> ||
                    process1_type::is_process_sequence) {
        tot += A_.getInverseLifetime(particle);
      }
    }
    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (is_decay_process_v<process2_type> ||
                    process2_type::is_process_sequence) {
        tot += B_.getInverseLifetime(particle);
      }
    }
    return tot;
  }

  template <typename TProcess1, typename TProcess2, int IndexStart, int IndexProcess1,
            int IndexProcess2>
  // select decay process
  template <typename TSecondaryView>
  inline ProcessReturn ProcessSequence<
      TProcess1, TProcess2, IndexStart, IndexProcess1,
      IndexProcess2>::selectDecay(TSecondaryView& view,
                                  [[maybe_unused]] InverseTimeType decay_inv_select,
                                  [[maybe_unused]] InverseTimeType decay_inv_sum) {

    // TODO: add check for decay_inv_select>decay_inv_tot

    if constexpr (is_process_v<process1_type>) { // to protect from further compiler
                                                 // errors if process1_type is invalid
      if constexpr (process1_type::is_process_sequence) {
        // if A_ is a process sequence --> check inside
        ProcessReturn const ret = A_.selectDecay(view, decay_inv_select, decay_inv_sum);
        // if A_ did succeed, stop routine here (not checking other static branch B_)
        if (ret != ProcessReturn::Ok) { return ret; }
      } else if constexpr (is_decay_process_v<process1_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += A_.getInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select <= decay_inv_sum) { // more pedagogical: rndm_select <
                                                 // decay_inv_sum / decay_inv_tot
          // interface checking on TProcess1
          static_assert(has_method_doDecay_v<TProcess1, void, TSecondaryView&>,
                        "TDerived has no method with correct signature \"void "
                        "doDecay(TSecondaryView&)\" required for "
                        "DecayProcess<TDerived>. ");

          A_.doDecay(view);
          return ProcessReturn::Decayed;
        }
      } // end branch A_
    }

    if constexpr (is_process_v<process2_type>) { // to protect from further compiler
                                                 // errors if process2_type is invalid
      if constexpr (process2_type::is_process_sequence) {
        // if B_ is a process sequence --> check inside
        return B_.selectDecay(view, decay_inv_select, decay_inv_sum);
      } else if constexpr (is_decay_process_v<process2_type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += B_.getInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select <= decay_inv_sum) {

          // interface checking on TProcess1
          static_assert(has_method_doDecay_v<TProcess2, void, TSecondaryView&>,
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

  /**
   * traits marker to identify objects containing any StackProcesses
   **/
  namespace detail {
    // need helper alias to achieve this:
    template <
        typename TProcess1, typename TProcess2,
        typename = typename std::enable_if_t<
            contains_stack_process_v<TProcess1> || is_stack_process_v<TProcess1> ||
                contains_stack_process_v<TProcess2> || is_stack_process_v<TProcess2>,
            int>>
    using enable_if_stack = ProcessSequence<TProcess1, TProcess2>;
  } // namespace detail

  template <typename TProcess1, typename TProcess2>
  struct contains_stack_process<detail::enable_if_stack<TProcess1, TProcess2>>
      : std::true_type {};

} // namespace corsika
