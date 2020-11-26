/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/process/ProcessTraits.h>
#include <corsika/process/BoundaryCrossingProcess.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/process/StackProcess.h>
#include <corsika/process/NullModel.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>

#include <cmath>
#include <limits>

namespace corsika::process {

  /**
     \class ProcessSequence

     A compile time static list of processes. The compiler will
     generate a new type based on template logic containing all the
     elements provided by the user.

     TProcess1 and TProcess2 must both be derived from BaseProcess,
     and are both references if possible (lvalue), otherwise (rvalue)
     they are just classes. This allows us to handle both, rvalue as
     well as lvalue Processes in the ProcessSequence.

     \comment Using CRTP pattern,
     https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
   */
  template <typename TProcess1, typename TProcess2 = NullModel>
  class ProcessSequence : public BaseProcess<ProcessSequence<TProcess1, TProcess2>> {

    using TProcess1type = typename std::decay_t<TProcess1>;
    using TProcess2type = typename std::decay_t<TProcess2>;

    static bool constexpr t1ProcSeq = is_process_sequence_v<TProcess1type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<TProcess2type>;

    static bool constexpr t1SwitchProcSeq = is_switch_process_sequence_v<TProcess1type>;
    static bool constexpr t2SwitchProcSeq = is_switch_process_sequence_v<TProcess2type>;

    // make sure only BaseProcess types TProcess1/2 are passed
    static_assert(std::is_base_of_v<BaseProcess<TProcess1type>, TProcess1type>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 1");
    static_assert(std::is_base_of_v<BaseProcess<TProcess2type>, TProcess2type>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 2");

    TProcess1 A_; // this is a reference, if possible
    TProcess2 B_; // this is a reference, if possible

  public:
    ProcessSequence(TProcess1 in_A, TProcess2 in_B)
        : A_(in_A)
        , B_(in_B) {}

    template <typename TParticle, typename TVTNType>
    EProcessReturn DoBoundaryCrossing(TParticle& particle, TVTNType const& from,
                                      TVTNType const& to) {
      EProcessReturn ret = EProcessReturn::eOk;

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess1type>,
                                      TProcess1type> ||
                    t1ProcSeq) {
        ret |= A_.DoBoundaryCrossing(particle, from, to);
      }

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess2type>,
                                      TProcess2type> ||
                    t2ProcSeq) {
        ret |= B_.DoBoundaryCrossing(particle, from, to);
      }

      return ret;
    }

    template <typename TParticle, typename TTrack>
    inline EProcessReturn DoContinuous(TParticle& particle, TTrack& vT) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        ret |= A_.DoContinuous(particle, vT);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        if (!isAbsorbed(ret)) { ret |= B_.DoContinuous(particle, vT); }
      }
      return ret;
    }

    template <typename TSecondaries>
    inline void DoSecondaries(TSecondaries& vS) {
      if constexpr (std::is_base_of_v<SecondariesProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        A_.DoSecondaries(vS);
      }
      if constexpr (std::is_base_of_v<SecondariesProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        B_.DoSecondaries(vS);
      }
    }

    /**
       The processes of type StackProcess do have an internal counter,
       so they can be exectuted only each N steps. Often these are
       "maintenacne processes" that do not need to run after each
       single step of the simulations. In the CheckStep function it is
       tested if either A_ or B_ are StackProcess and if they are due
       for execution.
     */
    inline bool CheckStep() {
      bool ret = false;
      if constexpr (std::is_base_of_v<StackProcess<TProcess1type>, TProcess1type> ||
                    (t1ProcSeq && !t1SwitchProcSeq)) {
        ret |= A_.CheckStep();
      }
      if constexpr (std::is_base_of_v<StackProcess<TProcess2type>, TProcess2type> ||
                    (t2ProcSeq && !t2SwitchProcSeq)) {
        ret |= B_.CheckStep();
      }
      return ret;
    }

    /**
       Execute the StackProcess-es in the ProcessSequence
     */
    template <typename TStack>
    inline void DoStack(TStack& stack) {
      if constexpr (std::is_base_of_v<StackProcess<TProcess1type>, TProcess1type> ||
                    (t1ProcSeq && !t1SwitchProcSeq)) {
        if (A_.CheckStep()) { A_.DoStack(stack); }
      }
      if constexpr (std::is_base_of_v<StackProcess<TProcess2type>, TProcess2type> ||
                    (t2ProcSeq && !t2SwitchProcSeq)) {
        if (B_.CheckStep()) { B_.DoStack(stack); }
      }
    }

    template <typename TParticle, typename TTrack>
    inline corsika::units::si::LengthType MaxStepLength(TParticle& particle,
                                                        TTrack& vTrack) {
      corsika::units::si::LengthType
          max_length = // if no other process in the sequence implements it
          std::numeric_limits<double>::infinity() * corsika::units::si::meter;

      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        corsika::units::si::LengthType const len = A_.MaxStepLength(particle, vTrack);
        max_length = std::min(max_length, len);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        corsika::units::si::LengthType const len = B_.MaxStepLength(particle, vTrack);
        max_length = std::min(max_length, len);
      }
      return max_length;
    }

    template <typename TParticle>
    inline corsika::units::si::GrammageType GetInteractionLength(TParticle&& particle) {
      return 1. / GetInverseInteractionLength(particle);
    }

    template <typename TParticle>
    inline corsika::units::si::InverseGrammageType GetInverseInteractionLength(
        TParticle&& particle) {
      using namespace corsika::units::si;

      InverseGrammageType tot = 0 * meter * meter / gram; // default value

      if constexpr (std::is_base_of_v<InteractionProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        tot += A_.GetInverseInteractionLength(particle);
      }
      if constexpr (std::is_base_of_v<InteractionProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        tot += B_.GetInverseInteractionLength(particle);
      }
      return tot;
    }

    template <typename TSecondaryView>
    inline EProcessReturn SelectInteraction(
        TSecondaryView& view,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_inv_select,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_inv_sum =
            corsika::units::si::InverseGrammageType::zero()) {

      // TODO: add check for lambda_inv_select>lambda_inv_tot

      if constexpr (t1ProcSeq) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret =
            A_.SelectInteraction(view, lambda_inv_select, lambda_inv_sum);
        // if A_ did succeed, stop routine. Not checking other static branch B_.
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<InteractionProcess<TProcess1type>,
                                             TProcess1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        const auto particle = view.parent();
        lambda_inv_sum += A_.GetInverseInteractionLength(particle);
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select <= lambda_inv_sum) {
          A_.DoInteraction(view);
          return EProcessReturn::eInteracted;
        }
      } // end branch A_

      if constexpr (t2ProcSeq) {
        // if B_ is a process sequence --> check inside
        return B_.SelectInteraction(view, lambda_inv_select, lambda_inv_sum);
      } else if constexpr (std::is_base_of_v<InteractionProcess<TProcess2type>,
                                             TProcess2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += B_.GetInverseInteractionLength(view.parent());
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select <= lambda_inv_sum) {
          B_.DoInteraction(view);
          return EProcessReturn::eInteracted;
        }
      } // end branch B_
      return EProcessReturn::eOk;
    }

    template <typename TParticle>
    inline corsika::units::si::TimeType GetLifetime(TParticle& particle) {
      return 1. / GetInverseLifetime(particle);
    }

    template <typename TParticle>
    inline corsika::units::si::InverseTimeType GetInverseLifetime(TParticle&& particle) {
      using namespace corsika::units::si;

      corsika::units::si::InverseTimeType tot = 0 / second; // default value

      if constexpr (std::is_base_of_v<DecayProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        tot += A_.GetInverseLifetime(particle);
      }
      if constexpr (std::is_base_of_v<DecayProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        tot += B_.GetInverseLifetime(particle);
      }
      return tot;
    }

    // select decay process
    template <typename TSecondaryView>
    inline EProcessReturn SelectDecay(
        TSecondaryView& view,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_inv_select,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_inv_sum =
            corsika::units::si::InverseTimeType::zero()) {

      // TODO: add check for decay_inv_select>decay_inv_tot

      if constexpr (t1ProcSeq) {
        // if A_ is a process sequence --> check inside
        const EProcessReturn ret = A_.SelectDecay(view, decay_inv_select, decay_inv_sum);
        // if A_ did succeed, stop routine here (not checking other static branch B_)
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<DecayProcess<TProcess1type>,
                                             TProcess1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += A_.GetInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select <= decay_inv_sum) { // more pedagogical: rndm_select <
                                                 // decay_inv_sum / decay_inv_tot
          A_.DoDecay(view);
          return EProcessReturn::eDecayed;
        }
      } // end branch A_

      if constexpr (t2ProcSeq) {
        // if B_ is a process sequence --> check inside
        return B_.SelectDecay(view, decay_inv_select, decay_inv_sum);
      } else if constexpr (std::is_base_of_v<DecayProcess<TProcess2type>,
                                             TProcess2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += B_.GetInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select <= decay_inv_sum) {
          B_.DoDecay(view);
          return EProcessReturn::eDecayed;
        }
      } // end branch B_
      return EProcessReturn::eOk;
    }
  };

  /**
   * \function sequence
   *
   * to construct ProcessSequences in a flexible and dynamic way the
   * `sequence` factory functions are provided
   *
   * Any objects of type
   *  - BaseProcess,
   *  - ContinuousProcess, and
   *  - Interaction/DecayProcess,
   *  - StackProcess,
   *  - SecondariesProcess
   * can be assembled into a ProcessSequence, all
   * combinatorics are allowed.

   * The sequence function checks that all its arguments are all of
   * types derived from BaseProcess. Also the ProcessSequence itself
   * is derived from type BaseProcess
   **/

  template <typename... TProcesses, typename TProcess1>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                        typename std::decay_t<TProcess1>>,
      ProcessSequence<TProcess1, decltype(sequence(std::declval<TProcesses>()...))>>
  sequence(TProcess1&& vA, TProcesses&&... vBs) {
    return ProcessSequence<TProcess1, decltype(sequence(std::declval<TProcesses>()...))>(
        vA, sequence(std::forward<TProcesses>(vBs)...));
  }

  template <typename TProcess1, typename TProcess2>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                        typename std::decay_t<TProcess1>> &&
          std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess2>>,
                            typename std::decay_t<TProcess2>>,
      ProcessSequence<TProcess1, TProcess2>>
  sequence(TProcess1&& vA, TProcess2&& vB) {
    return ProcessSequence<TProcess1, TProcess2>(vA, vB);
  }

  /**
   * also allow a single Process in ProcessSequence, accompany by
   * `NullModel`
   **/
  template <typename TProcess>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess>>,
                        typename std::decay_t<TProcess>>,
      ProcessSequence<TProcess, NullModel>>
  sequence(TProcess&& vA) {
    return ProcessSequence<TProcess, NullModel>(vA, NullModel());
  }

  /**
   * traits marker to identify objectas ProcessSequence
   **/
  template <typename TProcess1, typename TProcess2>
  struct is_process_sequence<ProcessSequence<TProcess1, TProcess2>> : std::true_type {
    // only switch on for BaseProcesses
    template <typename std::enable_if_t<
        std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                          typename std::decay_t<TProcess1>> &&
            std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess2>>,
                              typename std::decay_t<TProcess2>>,
        int>>
    is_process_sequence() {}
  };

  /**
   * traits marker to identify objects containing any StackProcesses
   **/
  namespace detail {
    // need helper alias to achieve this:
    template <typename TProcess1, typename TProcess2,
              typename = typename std::enable_if_t<
                  contains_stack_process_v<TProcess1> ||
                      std::is_base_of_v<StackProcess<typename std::decay_t<TProcess1>>,
                                        typename std::decay_t<TProcess1>> ||
                      contains_stack_process_v<TProcess2> ||
                      std::is_base_of_v<StackProcess<typename std::decay_t<TProcess2>>,
                                        typename std::decay_t<TProcess2>>,
                  int>>
    using enable_if_stack = ProcessSequence<TProcess1, TProcess2>;
  } // namespace detail

  template <typename TProcess1, typename TProcess2>
  struct contains_stack_process<detail::enable_if_stack<TProcess1, TProcess2>>
      : std::true_type {};

} // namespace corsika::process
