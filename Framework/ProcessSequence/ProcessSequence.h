
/**
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
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessReturn.h>
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
  struct is_process_sequence {
    static const bool value = false;
  };

  /**
     T1 and T2 are both references if possible (lvalue), otherwise
     (rvalue) they are just classes. This allows us to handle both,
     rvalue as well as lvalue Processes in the ProcessSequence.
   */
  template <typename T1, typename T2>
  class ProcessSequence : public BaseProcess<ProcessSequence<T1, T2> > {

    using T1type = typename std::decay<T1>::type;
    using T2type = typename std::decay<T2>::type;

  public:
    T1 A; // this is a reference, if possible
    T2 B; // this is a reference, if possible

    // ProcessSequence(ProcessSequence<T1,T2>&& v) : A(v.A), B(v.B) {}
    // ProcessSequence<T1,T2>& operator=(ProcessSequence<T1,T2>&& v) { A=v.A; B=v.B;
    // return *this; }

    ProcessSequence(T1 in_A, T2 in_B)
        : A(in_A)
        , B(in_B) {}

    // example for a trait-based call:
    // void Hello() const  { detail::CallHello<T1,T2>::Call(A, B); }

    template <typename Particle, typename Track, typename Stack>
    EProcessReturn DoContinuous(Particle& p, Track& t, Stack& s) {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (std::is_base_of<ContinuousProcess<T1type>, T1type>::value ||
                    is_process_sequence<T1>::value) {
        ret |= A.DoContinuous(p, t, s);
      }
      if constexpr (std::is_base_of<ContinuousProcess<T2type>, T2type>::value ||
                    is_process_sequence<T2>::value) {
        ret |= B.DoContinuous(p, t, s);
      }
      return ret;
    }

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle& p, Track& track) {
      corsika::units::si::LengthType
          max_length = // if no other process in the sequence implements it
          std::numeric_limits<double>::infinity() * corsika::units::si::meter;

      if constexpr (std::is_base_of<ContinuousProcess<T1type>, T1type>::value ||
                    is_process_sequence<T1>::value) {
        corsika::units::si::LengthType const len = A.MaxStepLength(p, track);
        max_length = std::min(max_length, len);
      }
      if constexpr (std::is_base_of<ContinuousProcess<T2type>, T2type>::value ||
                    is_process_sequence<T2>::value) {
        corsika::units::si::LengthType const len = B.MaxStepLength(p, track);
        max_length = std::min(max_length, len);
      }
      return max_length;
    }

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetTotalInteractionLength(Particle& p, Track& t) {
      return 1. / GetInverseInteractionLength(p, t);
    }

    template <typename Particle, typename Track>
    corsika::units::si::InverseGrammageType GetTotalInverseInteractionLength(Particle& p,
                                                                             Track& t) {
      return GetInverseInteractionLength(p, t);
    }

    template <typename Particle, typename Track>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(Particle& p,
                                                                        Track& t) {
      using namespace corsika::units::si;

      InverseGrammageType tot = 0 * meter * meter / gram;

      if constexpr (std::is_base_of<InteractionProcess<T1type>, T1type>::value ||
                    is_process_sequence<T1>::value) {
        tot += A.GetInverseInteractionLength(p, t);
      }
      if constexpr (std::is_base_of<InteractionProcess<T2type>, T2type>::value ||
                    is_process_sequence<T2>::value) {
        tot += B.GetInverseInteractionLength(p, t);
      }
      return tot;
    }

    template <typename Particle, typename Stack>
    EProcessReturn SelectInteraction(
        Particle& p, Stack& s, [[maybe_unused]]corsika::units::si::InverseGrammageType lambda_select,
        corsika::units::si::InverseGrammageType& lambda_inv_count) {

      if constexpr (is_process_sequence<T1type>::value) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret =
            A.SelectInteraction(p, s, lambda_select, lambda_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of<InteractionProcess<T1type>, T1type>::value) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_count += A.GetInverseInteractionLength(p, s);
        // check if we should execute THIS process and then EXIT
        if (lambda_select < lambda_inv_count) {
          A.DoInteraction(p, s);
          return EProcessReturn::eInteracted;
        }
      } // end branch A

      if constexpr (is_process_sequence<T2>::value) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret =
            B.SelectInteraction(p, s, lambda_select, lambda_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of<InteractionProcess<T2type>, T2type>::value) {
        // if this is not a ContinuousProcess --> evaluate probability
        lambda_inv_count += B.GetInverseInteractionLength(p, s);
        // check if we should execute THIS process and then EXIT
        if (lambda_select < lambda_inv_count) {
          B.DoInteraction(p, s);
          return EProcessReturn::eInteracted;
        }
      } // end branch A
      return EProcessReturn::eOk;
    }

    template <typename Particle>
    corsika::units::si::TimeType GetTotalLifetime(Particle& p) {
      return 1. / GetInverseLifetime(p);
    }

    template <typename Particle>
    corsika::units::si::InverseTimeType GetTotalInverseLifetime(Particle& p) {
      return GetInverseLifetime(p);
    }

    template <typename Particle>
    corsika::units::si::InverseTimeType GetInverseLifetime(Particle& p) {
      using namespace corsika::units::si;

      corsika::units::si::InverseTimeType tot = 0 / second;

      if constexpr (std::is_base_of<DecayProcess<T1type>, T1type>::value ||
                    is_process_sequence<T1>::value) {
        tot += A.GetInverseLifetime(p);
      }
      if constexpr (std::is_base_of<DecayProcess<T2type>, T2type>::value ||
                    is_process_sequence<T2>::value) {
        tot += B.GetInverseLifetime(p);
      }
      return tot;
    }

    // select decay process
    template <typename Particle, typename Stack>
    EProcessReturn SelectDecay(Particle& p, Stack& s,
                               [[maybe_unused]] corsika::units::si::InverseTimeType decay_select,
                               corsika::units::si::InverseTimeType& decay_inv_count) {
      if constexpr (is_process_sequence<T1>::value) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret = A.SelectDecay(p, s, decay_select, decay_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of<DecayProcess<T1type>, T1type>::value) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_count += A.GetInverseLifetime(p);
        // check if we should execute THIS process and then EXIT
        if (decay_select < decay_inv_count) { // more pedagogical: rndm_select <
                                              // decay_inv_count / decay_inv_tot
          A.DoDecay(p, s);
          return EProcessReturn::eDecayed;
        }
      } // end branch A

      if constexpr (is_process_sequence<T2>::value) {
        // if A is a process sequence --> check inside
        const EProcessReturn ret = B.SelectDecay(p, s, decay_select, decay_inv_count);
        // if A did succeed, stop routine
        if (ret != EProcessReturn::eOk) { return ret; }
      } else if constexpr (std::is_base_of<DecayProcess<T2type>, T2type>::value) {
        // if this is not a ContinuousProcess --> evaluate probability
        decay_inv_count += B.GetInverseLifetime(p);
        // check if we should execute THIS process and then EXIT
        if (decay_select < decay_inv_count) {
          B.DoDecay(p, s);
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
  inline auto operator<<(P1&& A, P2&& B) -> ProcessSequence<P1, P2> {
    return ProcessSequence<P1, P2>(A.GetRef(), B.GetRef());
  }

  /* #define OPSEQ(C1, C2) \ */
  /*   template < \ */
  /*       typename P1, typename P2, \ */
  /*       typename std::enable_if<is_process<typename std::decay<P1>::type>::value &&
   * \ */
  /*                               is_process<typename
   * std::decay<P2>::type>::value>::type...> \ */
  /*   inline auto operator+(P1&& A, P2&& B)->ProcessSequence<P1, P2> { \ */
  /*     return ProcessSequence<P1, P2>(A.GetRef(), B.GetRef()); \ */
  /*   } */

  /*   /\*template <typename T1, typename T2>				\ */
  /* inline ProcessSequence<T1, T2> operator%(C1<T1>& A, C2<T2>& B) {	\ */
  /* return ProcessSequence<T1, T2>(A.GetRef(), B.GetRef());	\ */
  /* }*\/ */

  /*   OPSEQ(BaseProcess, BaseProcess) */
  /*   OPSEQ(BaseProcess, InteractionProcess) */
  /*   OPSEQ(BaseProcess, ContinuousProcess) */
  /*   OPSEQ(BaseProcess, DecayProcess) */
  /*   OPSEQ(ContinuousProcess, BaseProcess) */
  /*   OPSEQ(ContinuousProcess, InteractionProcess) */
  /*   OPSEQ(ContinuousProcess, ContinuousProcess) */
  /*   OPSEQ(ContinuousProcess, DecayProcess) */
  /*   OPSEQ(InteractionProcess, BaseProcess) */
  /*   OPSEQ(InteractionProcess, InteractionProcess) */
  /*   OPSEQ(InteractionProcess, ContinuousProcess) */
  /*   OPSEQ(InteractionProcess, DecayProcess) */
  /*   OPSEQ(DecayProcess, BaseProcess) */
  /*   OPSEQ(DecayProcess, InteractionProcess) */
  /*   OPSEQ(DecayProcess, ContinuousProcess) */
  /*   OPSEQ(DecayProcess, DecayProcess) */

  /// marker to identify objectas ProcessSequence
  template <typename A, typename B>
  struct is_process_sequence<corsika::process::ProcessSequence<A, B> > {
    static const bool value = true;
  };

} // namespace corsika::process

#endif
