/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_ProcessSequence_h_
#define _include_ProcessSequence_h_

#include <corsika/process/BaseProcess.h>
#include <corsika/process/BoundaryCrossingProcess.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/process/StackProcess.h>
#include <corsika/units/PhysicalUnits.h>

#include <cmath>
#include <limits>
#include <type_traits>

namespace corsika::process {

  /**
     \class ProcessSequence

     A compile time static list of processes. The compiler will
     generate a new type based on template logic containing all the
     elements.

     \comment Using CRTP pattern,
     https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
   */

  // define a marker (trait class) to tag any class that qualifies as "Process" for the
  // "ProcessSequence"
  std::false_type is_process_impl(...);
  template <class T>
  using is_process = decltype(is_process_impl(std::declval<T*>()));

  // this is a marker to track which BaseProcess is also a ProcessSequence
  template <typename T>
  struct is_process_sequence : std::false_type {};

  template <typename T>
  bool constexpr is_process_sequence_v = is_process_sequence<T>::value;

  namespace switch_process {
    template <typename A, typename B>
    class SwitchProcess; // fwd-decl.
  }

  // to detect SwitchProcesses inside the ProcessSequence
  template <typename T>
  struct is_switch_process : std::false_type {};

  template <typename T>
  bool constexpr is_switch_process_v = is_switch_process<T>::value;

  template <typename A, typename B>
  struct is_switch_process<switch_process::SwitchProcess<A, B>> : std::true_type {};

  /**
     T1 and T2 are both references if possible (lvalue), otherwise
     (rvalue) they are just classes. This allows us to handle both,
     rvalue as well as lvalue Processes in the ProcessSequence.
   */
  template <typename T1, typename T2>
  class ProcessSequence : public BaseProcess<ProcessSequence<T1, T2>> {

    using T1type = typename std::decay<T1>::type;
    using T2type = typename std::decay<T2>::type;

    static bool constexpr t1ProcSeq = is_process_sequence_v<T1type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<T2type>;

    static bool constexpr t1SwitchProc = is_switch_process_v<T1type>;
    static bool constexpr t2SwitchProc = is_switch_process_v<T2type>;

  public:
    T1 A; // this is a reference, if possible
    T2 B; // this is a reference, if possible

    ProcessSequence(T1 in_A, T2 in_B)
        : A(in_A)
        , B(in_B) {}

    // example for a trait-based call:
    // void Hello() const  { detail::CallHello<T1,T2>::Call(A, B); }

    template <typename Particle, typename VTNType>
    EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                      VTNType const& to) {
      EProcessReturn ret = EProcessReturn::eOk;

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<T1type>, T1type> ||
                    t1ProcSeq) {
        ret |= A.DoBoundaryCrossing(p, from, to);
      }

      if constexpr (std::is_base_of_v<BoundaryCrossingProcess<T2type>, T2type> ||
                    t2ProcSeq) {
        ret |= B.DoBoundaryCrossing(p, from, to);
      }

      return ret;
    }

    template <typename TParticle, typename TTrack>
    EProcessReturn DoContinuous(TParticle& vP, TTrack& vT) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of_v<ContinuousProcess<T1type>, T1type> || t1ProcSeq) {
        ret |= A.DoContinuous(vP, vT);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<T2type>, T2type> || t2ProcSeq) {
        ret |= B.DoContinuous(vP, vT);
      }
      return ret;
    }

    template <typename TSecondaries>
    EProcessReturn DoSecondaries(TSecondaries& vS) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of_v<SecondariesProcess<T1type>, T1type> || t1ProcSeq) {
        ret |= A.DoSecondaries(vS);
      }
      if constexpr (std::is_base_of_v<SecondariesProcess<T2type>, T2type> || t2ProcSeq) {
        ret |= B.DoSecondaries(vS);
      }
      return ret;
    }

    /**
       The processes of type StackProcess do have an internal counter,
       so they can be exectuted only each N steps. Often these are
       "maintenacne processes" that do not need to run after each
       single step of the simulations. In the CheckStep function it is
       tested if either A or B are StackProcess and if they are due
       for execution.
     */
    bool CheckStep() {
      bool ret = false;
      if constexpr (std::is_base_of_v<StackProcess<T1type>, T1type> || t1ProcSeq) {
        ret |= A.CheckStep();
      }
      if constexpr (std::is_base_of_v<StackProcess<T2type>, T2type> || t2ProcSeq) {
        ret |= B.CheckStep();
      }
      return ret;
    }

    /**
       Execute the StackProcess-es in the ProcessSequence
     */
    template <typename TStack>
    EProcessReturn DoStack(TStack& vS) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of_v<StackProcess<T1type>, T1type> || t1ProcSeq) {
        if (A.CheckStep()) { ret |= A.DoStack(vS); }
      }
      if constexpr (std::is_base_of_v<StackProcess<T2type>, T2type> || t2ProcSeq) {
        if (B.CheckStep()) { ret |= B.DoStack(vS); }
      }
      return ret;
    }

    template <typename TParticle, typename TTrack>
    corsika::units::si::LengthType MaxStepLength(TParticle& vP, TTrack& vTrack) {
      corsika::units::si::LengthType
          max_length = // if no other process in the sequence implements it
          std::numeric_limits<double>::infinity() * corsika::units::si::meter;

      if constexpr (std::is_base_of_v<ContinuousProcess<T1type>, T1type> || t1ProcSeq) {
        corsika::units::si::LengthType const len = A.MaxStepLength(vP, vTrack);
        max_length = std::min(max_length, len);
      }
      if constexpr (std::is_base_of_v<ContinuousProcess<T2type>, T2type> || t2ProcSeq) {
        corsika::units::si::LengthType const len = B.MaxStepLength(vP, vTrack);
        max_length = std::min(max_length, len);
      }
      return max_length;
    }

    template <typename TParticle>
    corsika::units::si::GrammageType GetTotalInteractionLength(TParticle& vP) {
      return 1. / GetInverseInteractionLength(vP);
    }

    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetTotalInverseInteractionLength(
        TParticle& vP) {
      return GetInverseInteractionLength(vP);
    }

    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(TParticle& vP) {
      using namespace corsika::units::si;

      InverseGrammageType tot = 0 * meter * meter / gram;

      if constexpr (std::is_base_of_v<InteractionProcess<T1type>, T1type> || t1ProcSeq ||
                    t1SwitchProc) {
        tot += A.GetInverseInteractionLength(vP);
      }
      if constexpr (std::is_base_of_v<InteractionProcess<T2type>, T2type> || t2ProcSeq ||
                    t2SwitchProc) {
        tot += B.GetInverseInteractionLength(vP);
      }
      return tot;
    }

    template <typename TParticle, typename TSecondaries>
    EProcessReturn SelectInteraction(
        TParticle& vP, TSecondaries& vS,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_select,
        corsika::units::si::InverseGrammageType& lambda_inv_count) {

      if constexpr (t1ProcSeq || t1SwitchProc) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret =
            A.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<InteractionProcess<T1type>, T1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_count += A.GetInverseInteractionLength(vP);
        // check if we should execute THIS process and then EXIT
        if (lambda_select < lambda_inv_count) {
          A.DoInteraction(vS);
          return EProcessReturn::eInteracted;
        }
      } // end branch A

      if constexpr (t2ProcSeq || t2SwitchProc) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret =
            B.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<InteractionProcess<T2type>, T2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_count += B.GetInverseInteractionLength(vP);
        // check if we should execute THIS process and then EXIT
        if (lambda_select < lambda_inv_count) {
          B.DoInteraction(vS);
          return EProcessReturn::eInteracted;
        }
      } // end branch A
      return EProcessReturn::eOk;
    }

    template <typename TParticle>
    corsika::units::si::TimeType GetTotalLifetime(TParticle& p) {
      return 1. / GetInverseLifetime(p);
    }

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetTotalInverseLifetime(TParticle& p) {
      return GetInverseLifetime(p);
    }

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetInverseLifetime(TParticle& p) {
      using namespace corsika::units::si;

      corsika::units::si::InverseTimeType tot = 0 / second;

      if constexpr (std::is_base_of_v<DecayProcess<T1type>, T1type> || t1ProcSeq) {
        tot += A.GetInverseLifetime(p);
      }
      if constexpr (std::is_base_of_v<DecayProcess<T2type>, T2type> || t2ProcSeq) {
        tot += B.GetInverseLifetime(p);
      }
      return tot;
    }

    // select decay process
    template <typename TParticle, typename TSecondaries>
    EProcessReturn SelectDecay(
        TParticle& vP, TSecondaries& vS,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_select,
        corsika::units::si::InverseTimeType& decay_inv_count) {
      if constexpr (t1ProcSeq) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret = A.SelectDecay(vP, vS, decay_select, decay_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<DecayProcess<T1type>, T1type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_count += A.GetInverseLifetime(vP);
        // check if we should execute THIS process and then EXIT
        if (decay_select < decay_inv_count) { // more pedagogical: rndm_select <
                                              // decay_inv_count / decay_inv_tot
          A.DoDecay(vS);
          return EProcessReturn::eDecayed;
        }
      } // end branch A

      if constexpr (t2ProcSeq) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret = B.SelectDecay(vP, vS, decay_select, decay_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of_v<DecayProcess<T2type>, T2type>) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_count += B.GetInverseLifetime(vP);
        // check if we should execute THIS process and then EXIT
        if (decay_select < decay_inv_count) {
          B.DoDecay(vS);
          return EProcessReturn::eDecayed;
        }
      } // end branch B
      return EProcessReturn::eOk;
    }

    void Init() {
      A.Init();
      B.Init();
    }
  };

  /// the << operator assembles many BaseProcess, ContinuousProcess, and
  /// Interaction/DecayProcess objects into a ProcessSequence, all combinatorics
  /// must be allowed, this is why we define a macro to define all
  /// combinations here:

  template <
      typename P1, typename P2,
      typename std::enable_if<is_process<typename std::decay<P1>::type>::value &&
                              is_process<typename std::decay<P2>::type>::value>::type...>
  inline auto operator<<(P1&& vA, P2&& vB) -> ProcessSequence<P1, P2> {
    return ProcessSequence<P1, P2>(vA.GetRef(), vB.GetRef());
  }

  /// marker to identify objectas ProcessSequence
  template <typename A, typename B>
  struct is_process_sequence<corsika::process::ProcessSequence<A, B>> : std::true_type {};

} // namespace corsika::process

#endif
