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

#include <cmath>
#include <limits>

namespace corsika::process {

  // enum for the process switch selection: identify if First or
  // Second process branch should be used.
  enum class SwitchResult { First, Second };

  /**
     \class SwitchProcessSequence

     A compile time static list of processes that uses an internal
     TSelect class to switch between different versions of processes
     (or process sequence).

     TProcess1 and TProcess2 must be derived from BaseProcess and are
     both references if possible (lvalue), otherwise (rvalue) they are
     just classes. This allows us to handle both, rvalue as well as
     lvalue Processes in the SwitchProcessSequence.

     TSelect has to implement a `select(const Particle&)` and has to
     return either SwitchResult::First or SwitchResult::Second. Note:
     TSelect may absolutely also use random numbers to sample between
     its results. This can be used to achieve arbitrarily smooth
     transition or mixtures of processes.

     Warning: do not put StackProcess into a SwitchProcessSequence
     since this makes no sense. The StackProcess acts on an entire
     particle stack and not on indiviidual particles.

     \comment See also class ProcessSequence
  **/

  template <typename TProcess1, typename TProcess2, typename TSelect>
  class SwitchProcessSequence
      : public BaseProcess<SwitchProcessSequence<TProcess1, TProcess2, TSelect>> {

    using TProcess1type = typename std::decay<TProcess1>::type;
    using TProcess2type = typename std::decay<TProcess2>::type;

    static bool constexpr t1ProcSeq = is_process_sequence_v<TProcess1type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<TProcess2type>;

    TSelect select_; // this is a reference, if possible

  public:
    TProcess1 A; // this is a reference, if possible
    TProcess2 B; // this is a reference, if possible

    SwitchProcessSequence(TProcess1 in_A, TProcess2 in_B, TSelect sel)
        : select_(sel)
        , A(in_A)
        , B(in_B) {}

    template <typename Particle, typename VTNType>
    EProcessReturn DoBoundaryCrossing(Particle& particle, VTNType const& from,
                                      VTNType const& to) {

      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess1type>,
                                          TProcess1type> ||
                        t1ProcSeq) {
            return A.DoBoundaryCrossing(particle, from, to);
          }
          break;
        }
        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<BoundaryCrossingProcess<TProcess2type>,
                                          TProcess2type> ||
                        t2ProcSeq) {
            return B.DoBoundaryCrossing(particle, from, to);
          }
          break;
        }
      }
      return EProcessReturn::eOk;
    }

    template <typename TParticle, typename TTrack>
    inline EProcessReturn DoContinuous(TParticle& particle, TTrack& vT) {
      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<ContinuousProcess<TProcess1type>,
                                          TProcess1type> ||
                        t1ProcSeq) {
            return A.DoContinuous(particle, vT);
          }
          break;
        }
        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>,
                                          TProcess2type> ||
                        t2ProcSeq) {
            return B.DoContinuous(particle, vT);
          }
          break;
        }
      }
      return EProcessReturn::eOk;
    }

    template <typename TSecondaries>
    inline void DoSecondaries(TSecondaries& vS) {
      const auto& particle = vS.parent();
      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<SecondariesProcess<TProcess1type>,
                                          TProcess1type> ||
                        t1ProcSeq) {
            A.DoSecondaries(vS);
          }
          break;
        }
        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<SecondariesProcess<TProcess2type>,
                                          TProcess2type> ||
                        t2ProcSeq) {
            B.DoSecondaries(vS);
          }
          break;
        }
      }
    }

    template <typename TParticle, typename TTrack>
    inline corsika::units::si::LengthType MaxStepLength(TParticle& particle,
                                                        TTrack& vTrack) {

      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<ContinuousProcess<TProcess1type>,
                                          TProcess1type> ||
                        t1ProcSeq) {
            return A.MaxStepLength(particle, vTrack);
          }
          break;
        }
        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<ContinuousProcess<TProcess2type>,
                                          TProcess2type> ||
                        t2ProcSeq) {
            return B.MaxStepLength(particle, vTrack);
          }
          break;
        }
      }

      // if no other process in the sequence implements it
      return std::numeric_limits<double>::infinity() * corsika::units::si::meter;
    }

    template <typename TParticle>
    inline corsika::units::si::GrammageType GetInteractionLength(TParticle&& particle) {
      return 1. / GetInverseInteractionLength(particle);
    }

    template <typename TParticle>
    inline corsika::units::si::InverseGrammageType GetInverseInteractionLength(
        TParticle&& particle) {
      using namespace corsika::units::si;

      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<InteractionProcess<TProcess1type>,
                                          TProcess1type> ||
                        t1ProcSeq) {
            return A.GetInverseInteractionLength(particle);
          }
          break;
        }
        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<InteractionProcess<TProcess2type>,
                                          TProcess2type> ||
                        t2ProcSeq) {
            return B.GetInverseInteractionLength(particle);
          }
          break;
        }
      }
      return 0 * meter * meter / gram; // default value
    }

    template <typename TSecondaryView>
    inline EProcessReturn SelectInteraction(
        TSecondaryView& view,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_inv_select,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_inv_sum =
            corsika::units::si::InverseGrammageType::zero()) {

      switch (select_.select(view.parent())) {
        case SwitchResult::First: {
          if constexpr (t1ProcSeq) {
            // if A is a process sequence --> check inside
            const EProcessReturn ret =
                A.SelectInteraction(view, lambda_inv_select, lambda_inv_sum);
            // if A did succeed, stop routine. Not checking other static branch B.
            if (ret != EProcessReturn::eOk) { return ret; }
          } else if constexpr (std::is_base_of_v<InteractionProcess<TProcess1type>,
                                                 TProcess1type>) {
            // if this is not a ContinuousProcess --> evaluate probability
            lambda_inv_sum += A.GetInverseInteractionLength(view.parent());
            // check if we should execute THIS process and then EXIT
            if (lambda_inv_select < lambda_inv_sum) {
              A.DoInteraction(view);
              return EProcessReturn::eInteracted;
            }
          } // end branch A
          break;
        }

        case SwitchResult::Second: {

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
          break;
        }
      }
      return EProcessReturn::eOk;
    }

    template <typename TParticle>
    inline corsika::units::si::TimeType GetLifetime(TParticle&& particle) {
      return 1. / GetInverseLifetime(particle);
    }

    template <typename TParticle>
    inline corsika::units::si::InverseTimeType GetInverseLifetime(TParticle&& particle) {
      using namespace corsika::units::si;

      switch (select_.select(particle)) {
        case SwitchResult::First: {
          if constexpr (std::is_base_of_v<DecayProcess<TProcess1type>, TProcess1type> ||
                        t1ProcSeq) {
            return A.GetInverseLifetime(particle);
          }
          break;
        }

        case SwitchResult::Second: {
          if constexpr (std::is_base_of_v<DecayProcess<TProcess2type>, TProcess2type> ||
                        t2ProcSeq) {
            return B.GetInverseLifetime(particle);
          }
          break;
        }
      }
      return 0 / second; // default value
    }

    // select decay process
    template <typename TSecondaryView>
    inline EProcessReturn SelectDecay(
        TSecondaryView& view,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_inv_select,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_inv_sum =
            corsika::units::si::InverseTimeType::zero()) {

      switch (select_.select(view.parent())) {
        case SwitchResult::First: {
          if constexpr (t1ProcSeq) {
            // if A is a process sequence --> check inside
            const EProcessReturn ret =
                A.SelectDecay(view, decay_inv_select, decay_inv_sum);
            // if A did succeed, stop routine here (not checking other static branch B)
            if (ret != EProcessReturn::eOk) { return ret; }
          } else if constexpr (std::is_base_of_v<DecayProcess<TProcess1type>,
                                                 TProcess1type>) {
            // if this is not a ContinuousProcess --> evaluate probability
            decay_inv_sum += A.GetInverseLifetime(view.parent());
            // check if we should execute THIS process and then EXIT
            if (decay_inv_select < decay_inv_sum) {
              // more pedagogical: rndm_select < decay_inv_sum / decay_inv_tot
              A.DoDecay(view);
              return EProcessReturn::eDecayed;
            }
          } // end branch A
          break;
        }

        case SwitchResult::Second: {

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
          break;
        }
      }
      return EProcessReturn::eOk;
    }
  };

  // the method `select(proc1,proc1,selector)` assembles many
  // BaseProcesses, and ProcessSequences into a SwitchProcessSequence,
  // all combinatorics must be allowed, this is why we define a macro
  // to define all combinations here:

  // Both, Processes1 and Processes2, must derive from BaseProcesses

  template <typename TProcess1, typename TProcess2, typename TSelect>
  inline typename std::enable_if<
      std::is_base_of<BaseProcess<typename std::decay<TProcess1>::type>,
                      typename std::decay<TProcess1>::type>::value &&
          std::is_base_of<BaseProcess<typename std::decay<TProcess2>::type>,
                          typename std::decay<TProcess2>::type>::value,
      SwitchProcessSequence<TProcess1, TProcess2, TSelect>>::type
  select(TProcess1&& vA, TProcess2&& vB, TSelect selector) {
    return SwitchProcessSequence<TProcess1, TProcess2, TSelect>(vA, vB, selector);
  }

  /// traits marker to identify objectas ProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_process_sequence<
      corsika::process::SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

  /// traits marker to identify objectas SwitchProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_switch_process_sequence<
      corsika::process::SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

} // namespace corsika::process
