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
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>

#include <cmath>
#include <limits>

namespace corsika::process {

  /**
     \class ProcessSequence

     A compile time static list of processes. The compiler will
     generate a new type based on template logic containing all the
     elements.

     TProcess1 and TProcess2 must both be derived from BaseProcess,
     and are both references if possible (lvalue), otherwise (rvalue)
     they are just classes. This allows us to handle both, rvalue as
     well as lvalue Processes in the ProcessSequence.

     \comment Using CRTP pattern,
     https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
   */
  template <typename TProcess1, typename TProcess2>
  class ProcessSequence : public BaseProcess<ProcessSequence<TProcess1, TProcess2>> {

    using TProcess1type = typename std::decay<TProcess1>::type;
    using TProcess2type = typename std::decay<TProcess2>::type;

    static bool constexpr t1ProcSeq = is_process_sequence_v<TProcess1type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<TProcess2type>;

    static bool constexpr t1SwitchProcSeq = is_switch_process_sequence_v<TProcess1type>;
    static bool constexpr t2SwitchProcSeq = is_switch_process_sequence_v<TProcess2type>;

  protected:
    TProcess1 A; // this is a reference, if possible
    TProcess2 B; // this is a reference, if possible

  public:
    ProcessSequence(TProcess1 in_A, TProcess2 in_B)
        : A(in_A)
        , B(in_B) {}

    template <typename Particle, typename VTNType>
    EProcessReturn DoBoundaryCrossing(Particle& particle, VTNType const& from,
                                      VTNType const& to) {
      EProcessReturn ret = EProcessReturn::eOk;

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess1type>,
                                      TProcess1type> ||
                    t1ProcSeq) {
        ret |= A.DoBoundaryCrossing(particle, from, to);
      }

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess2type>,
                                      TProcess2type> ||
                    t2ProcSeq) {
        ret |= B.DoBoundaryCrossing(particle, from, to);
      }

      return ret;
    }

    template <typename TParticle, typename TTrack>
    inline EProcessReturn DoContinuous(TParticle& particle, TTrack& vT) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        ret |= A.DoContinuous(particle, vT);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        if (!isAbsorbed(ret)) { ret |= B.DoContinuous(particle, vT); }
      }
      return ret;
    }

    template <typename TSecondaries>
    inline void DoSecondaries(TSecondaries& vS) {
      if constexpr (std::is_base_of_v<SecondariesProcess<TProcess1type>, TProcess1type> ||
                    t1ProcSeq) {
        A.DoSecondaries(vS);
      }
      if constexpr (std::is_base_of_v<SecondariesProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        B.DoSecondaries(vS);
      }
    }

    /**
       The processes of type StackProcess do have an internal counter,
       so they can be exectuted only each N steps. Often these are
       "maintenacne processes" that do not need to run after each
       single step of the simulations. In the CheckStep function it is
       tested if either A or B are StackProcess and if they are due
       for execution.
     */
    inline bool CheckStep() {
      bool ret = false;
      if constexpr (std::is_base_of_v<StackProcess<TProcess1type>, TProcess1type> ||
                    (t1ProcSeq && !t1SwitchProcSeq)) {
        ret |= A.CheckStep();
      }
      if constexpr (std::is_base_of_v<StackProcess<TProcess2type>, TProcess2type> ||
                    (t2ProcSeq && !t2SwitchProcSeq)) {
        ret |= B.CheckStep();
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
        if (A.CheckStep()) { A.DoStack(stack); }
      }
      if constexpr (std::is_base_of_v<StackProcess<TProcess2type>, TProcess2type> ||
                    (t2ProcSeq && !t2SwitchProcSeq)) {
        if (B.CheckStep()) { B.DoStack(stack); }
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
        corsika::units::si::LengthType const len = A.MaxStepLength(particle, vTrack);
        max_length = std::min(max_length, len);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        corsika::units::si::LengthType const len = B.MaxStepLength(particle, vTrack);
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
        tot += A.GetInverseInteractionLength(particle);
      }
      if constexpr (std::is_base_of_v<InteractionProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        tot += B.GetInverseInteractionLength(particle);
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
            A.SelectInteraction(view, lambda_inv_select, lambda_inv_sum);
        // if A did succeed, stop routine. Not checking other static branch B.
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<InteractionProcess<TProcess1type>,
                                             TProcess1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        const auto particle = view.parent();
        lambda_inv_sum += A.GetInverseInteractionLength(particle);
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select < lambda_inv_sum) {
          A.DoInteraction(view);
          return EProcessReturn::eInteracted;
        }
      } // end branch A

      if constexpr (t2ProcSeq) {
        // if B is a process sequence --> check inside
        return B.SelectInteraction(view, lambda_inv_select, lambda_inv_sum);
      } else if constexpr (std::is_base_of_v<InteractionProcess<TProcess2type>,
                                             TProcess2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_sum += B.GetInverseInteractionLength(view.parent());
        // check if we should execute THIS process and then EXIT
        if (lambda_inv_select < lambda_inv_sum) {
          B.DoInteraction(view);
          return EProcessReturn::eInteracted;
        }
      } // end branch B
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
        tot += A.GetInverseLifetime(particle);
      }
      if constexpr (std::is_base_of_v<DecayProcess<TProcess2type>, TProcess2type> ||
                    t2ProcSeq) {
        tot += B.GetInverseLifetime(particle);
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
        // if A is a process sequence --> check inside
        const EProcessReturn ret = A.SelectDecay(view, decay_inv_select, decay_inv_sum);
        // if A did succeed, stop routine here (not checking other static branch B)
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<DecayProcess<TProcess1type>,
                                             TProcess1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += A.GetInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select < decay_inv_sum) { // more pedagogical: rndm_select <
                                                // decay_inv_sum / decay_inv_tot
          A.DoDecay(view);
          return EProcessReturn::eDecayed;
        }
      } // end branch A

      if constexpr (t2ProcSeq) {
        // if B is a process sequence --> check inside
        return B.SelectDecay(view, decay_inv_select, decay_inv_sum);
      } else if constexpr (std::is_base_of_v<DecayProcess<TProcess2type>,
                                             TProcess2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_sum += B.GetInverseLifetime(view.parent());
        // check if we should execute THIS process and then EXIT
        if (decay_inv_select < decay_inv_sum) {
          B.DoDecay(view);
          return EProcessReturn::eDecayed;
        }
      } // end branch B
      return EProcessReturn::eOk;
    }
  };

  // the % operator assembles many BaseProcess, ContinuousProcess, and
  // Interaction/DecayProcess objects into a ProcessSequence, all combinatorics
  // must be allowed, this is why we define a macro to define all
  // combinations here:

  // enable the % operator to construct ProcessSequence from two
  // Processes, only if both, Processes1 and Processes2, derive from
  // BaseProcesses

  template <typename TProcess1, typename TProcess2>
  inline typename std::enable_if<
      std::is_base_of<BaseProcess<typename std::decay<TProcess1>::type>,
                      typename std::decay<TProcess1>::type>::value &&
          std::is_base_of<BaseProcess<typename std::decay<TProcess2>::type>,
                          typename std::decay<TProcess2>::type>::value,
      ProcessSequence<TProcess1, TProcess2>>::type
  operator%(TProcess1&& vA, TProcess2&& vB) {
    return ProcessSequence<TProcess1, TProcess2>(vA, vB);
  }

  /// traits marker to identify objectas ProcessSequence
  template <typename TProcess1, typename TProcess2>
  struct is_process_sequence<corsika::process::ProcessSequence<TProcess1, TProcess2>>
      : std::true_type {};

} // namespace corsika::process
