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

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle, typename TVTNType>
  inline ProcessReturn
  SwitchProcessSequence<TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
                        IndexProcess2>::doBoundaryCrossing(TParticle& particle,
                                                           TVTNType const& from,
                                                           TVTNType const& to) {
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<BoundaryCrossingProcess<process1_type>,
                                        process1_type> ||
                      t1ProcSeq) {
          return A_.doBoundaryCrossing(particle, from, to);
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<BoundaryCrossingProcess<process2_type>,
                                        process2_type> ||
                      t2ProcSeq) {
          return B_.doBoundaryCrossing(particle, from, to);
        }
        break;
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ProcessReturn SwitchProcessSequence<
      TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
      IndexProcess2>::doContinuous(TParticle& particle, TTrack& vT,
                                   ContinuousProcessIndex const idLimit) {
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) { return A_.doContinuous(particle, vT, idLimit); }
        if constexpr (is_continuous_process_v<process1_type>) {
          return A_.doContinuous(particle, vT,
                                 idLimit == ContinuousProcessIndex(IndexProcess1));
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (t2ProcSeq) { return B_.doContinuous(particle, vT, idLimit); }
        if constexpr (is_continuous_process_v<process2_type>) {
          return B_.doContinuous(particle, vT,
                                 idLimit == ContinuousProcessIndex(IndexProcess2));
        }
        break;
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TSecondaries>
  inline void
  SwitchProcessSequence<TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
                        IndexProcess2>::doSecondaries(TSecondaries& vS) {
    const auto& particle = vS.parent();
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<SecondariesProcess<process1_type>,
                                        process1_type> ||
                      t1ProcSeq) {
          A_.doSecondaries(vS);
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<SecondariesProcess<process2_type>,
                                        process2_type> ||
                      t2ProcSeq) {
          B_.doSecondaries(vS);
        }
        break;
      }
    }
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle, typename TTrack>
  inline ContinuousProcessStepLength
  SwitchProcessSequence<TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
                        IndexProcess2>::getMaxStepLength(TParticle& particle,
                                                         TTrack& vTrack) {
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) { return A_.getMaxStepLength(particle, vTrack); }
        if constexpr (is_continuous_process_v<process1_type>) {
          return ContinuousProcessStepLength(A_.getMaxStepLength(particle, vTrack),
                                             ContinuousProcessIndex(IndexProcess1));
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (t2ProcSeq) { return B_.getMaxStepLength(particle, vTrack); }
        if constexpr (is_continuous_process_v<process2_type>) {
          return ContinuousProcessStepLength(B_.getMaxStepLength(particle, vTrack),
                                             ContinuousProcessIndex(IndexProcess2));
        }
        break;
      }
    }

    // if no other process in the sequence implements it
    return ContinuousProcessStepLength(std::numeric_limits<double>::infinity() * meter);
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle>
  inline InverseGrammageType SwitchProcessSequence<
      TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
      IndexProcess2>::getInverseInteractionLength(TParticle&& particle) {

    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<InteractionProcess<process1_type>,
                                        process1_type> ||
                      t1ProcSeq) {
          return A_.getInverseInteractionLength(particle);
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<InteractionProcess<process2_type>,
                                        process2_type> ||
                      t2ProcSeq) {
          return B_.getInverseInteractionLength(particle);
        }
        break;
      }
    }
    return 0 * meter * meter / gram; // default value
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TSecondaryView>
  inline ProcessReturn SwitchProcessSequence<TProcess1, TProcess2, TSelect, IndexStart,
                                             IndexProcess1, IndexProcess2>::
      selectInteraction(TSecondaryView& view,
                        [[maybe_unused]] InverseGrammageType lambda_inv_select,
                        [[maybe_unused]] InverseGrammageType lambda_inv_sum) {
    switch (select_(view.parent())) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) {
          // if A_ is a process sequence --> check inside
          ProcessReturn const ret =
              A_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
          // if A_ did succeed, stop routine. Not checking other static branch B_.
          if (ret != ProcessReturn::Ok) { return ret; }
        } else if constexpr (std::is_base_of_v<InteractionProcess<process1_type>,
                                               process1_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          lambda_inv_sum += A_.getInverseInteractionLength(view.parent());
          // check if we should execute THIS process and then EXIT
          if (lambda_inv_select < lambda_inv_sum) {
            A_.doInteraction(view);
            return ProcessReturn::Interacted;
          }
        } // end branch A_
        break;
      }

      case SwitchResult::Second: {

        if constexpr (t2ProcSeq) {
          // if B_ is a process sequence --> check inside
          return B_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
        } else if constexpr (std::is_base_of_v<InteractionProcess<process2_type>,
                                               process2_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          lambda_inv_sum += B_.getInverseInteractionLength(view.parent());
          // check if we should execute THIS process and then EXIT
          if (lambda_inv_select < lambda_inv_sum) {
            B_.doInteraction(view);
            return ProcessReturn::Interacted;
          }
        } // end branch B_
        break;
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  template <typename TParticle>
  inline InverseTimeType
  SwitchProcessSequence<TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
                        IndexProcess2>::getInverseLifetime(TParticle&& particle) {

    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<DecayProcess<process1_type>, process1_type> ||
                      t1ProcSeq) {
          return A_.getInverseLifetime(particle);
        }
        break;
      }

      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<DecayProcess<process2_type>, process2_type> ||
                      t2ProcSeq) {
          return B_.getInverseLifetime(particle);
        }
        break;
      }
    }
    return 0 / second; // default value
  }

  template <typename TProcess1, typename TProcess2, typename TSelect, int IndexStart,
            int IndexProcess1, int IndexProcess2>
  // select decay process
  template <typename TSecondaryView>
  inline ProcessReturn SwitchProcessSequence<
      TProcess1, TProcess2, TSelect, IndexStart, IndexProcess1,
      IndexProcess2>::selectDecay(TSecondaryView& view,
                                  [[maybe_unused]] InverseTimeType decay_inv_select,
                                  [[maybe_unused]] InverseTimeType decay_inv_sum) {
    switch (select_(view.parent())) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) {
          // if A_ is a process sequence --> check inside
          ProcessReturn const ret = A_.selectDecay(view, decay_inv_select, decay_inv_sum);
          // if A_ did succeed, stop routine here (not checking other static branch B_)
          if (ret != ProcessReturn::Ok) { return ret; }
        } else if constexpr (std::is_base_of_v<DecayProcess<process1_type>,
                                               process1_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          decay_inv_sum += A_.getInverseLifetime(view.parent());
          // check if we should execute THIS process and then EXIT
          if (decay_inv_select < decay_inv_sum) {
            // more pedagogical: rndm_select < decay_inv_sum / decay_inv_tot
            A_.doDecay(view);
            return ProcessReturn::Decayed;
          }
        } // end branch A_
        break;
      }

      case SwitchResult::Second: {

        if constexpr (t2ProcSeq) {
          // if B_ is a process sequence --> check inside
          return B_.selectDecay(view, decay_inv_select, decay_inv_sum);
        } else if constexpr (std::is_base_of_v<DecayProcess<process2_type>,
                                               process2_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          decay_inv_sum += B_.getInverseLifetime(view.parent());
          // check if we should execute THIS process and then EXIT
          if (decay_inv_select < decay_inv_sum) {
            B_.doDecay(view);
            return ProcessReturn::Decayed;
          }
        } // end branch B_
        break;
      }
    }
    return ProcessReturn::Ok;
  }

  /*
  /// traits marker to identify objectas ProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_process_sequence<ProcessSequence<typename std::decay_t<TProcess1>,
                                                   typename std::decay_t<TProcess2>,
                                                   typename std::decay_t<TSelect>>>
      : std::true_type {};
  */
  /// traits marker to identify objectas ProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_process_sequence<SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_process_sequence<SwitchProcessSequence<TProcess1, TProcess2, TSelect>&>
      : std::true_type {};

  /// traits marker to identify objectas SwitchProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_switch_process_sequence<SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

} // namespace corsika
