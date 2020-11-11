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

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TParticle, typename TVTNType>
  EProcessReturn SwitchProcessSequence<TProcess1, TProcess2, TSelect>::doBoundaryCrossing(
      TParticle& particle, TVTNType const& from, TVTNType const& to) {
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
    return EProcessReturn::eOk;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TParticle, typename TTrack>
  inline EProcessReturn
  SwitchProcessSequence<TProcess1, TProcess2, TSelect>::doContinuous(TParticle& particle,
                                                                     TTrack& vT) {
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<ContinuousProcess<process1_type>,
                                        process1_type> ||
                      t1ProcSeq) {
          return A_.doContinuous(particle, vT);
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<ContinuousProcess<process2_type>,
                                        process2_type> ||
                      t2ProcSeq) {
          return B_.doContinuous(particle, vT);
        }
        break;
      }
    }
    return EProcessReturn::eOk;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TSecondaries>
  inline void SwitchProcessSequence<TProcess1, TProcess2, TSelect>::doSecondaries(
      TSecondaries& vS) {
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

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TParticle, typename TTrack>
  inline LengthType SwitchProcessSequence<TProcess1, TProcess2, TSelect>::maxStepLength(
      TParticle& particle, TTrack& vTrack) {
    switch (select_(particle)) {
      case SwitchResult::First: {
        if constexpr (std::is_base_of_v<ContinuousProcess<process1_type>,
                                        process1_type> ||
                      t1ProcSeq) {
          return A_.maxStepLength(particle, vTrack);
        }
        break;
      }
      case SwitchResult::Second: {
        if constexpr (std::is_base_of_v<ContinuousProcess<process2_type>,
                                        process2_type> ||
                      t2ProcSeq) {
          return B_.maxStepLength(particle, vTrack);
        }
        break;
      }
    }

    // if no other process in the sequence implements it
    return std::numeric_limits<double>::infinity() * meter;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TParticle>
  inline InverseGrammageType
  SwitchProcessSequence<TProcess1, TProcess2, TSelect>::getInverseInteractionLength(
      TParticle&& particle) {

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

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TSecondaryView>
  inline EProcessReturn
  SwitchProcessSequence<TProcess1, TProcess2, TSelect>::selectInteraction(
      TSecondaryView& view, [[maybe_unused]] InverseGrammageType lambda_inv_select,
      [[maybe_unused]] InverseGrammageType lambda_inv_sum) {
    switch (select_(view.parent())) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) {
          // if A_ is a process sequence --> check inside
          EProcessReturn const ret =
              A_.selectInteraction(view, lambda_inv_select, lambda_inv_sum);
          // if A_ did succeed, stop routine. Not checking other static branch B_.
          if (ret != EProcessReturn::eOk) { return ret; }
        } else if constexpr (std::is_base_of_v<InteractionProcess<process1_type>,
                                               process1_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          lambda_inv_sum += A_.getInverseInteractionLength(view.parent());
          // check if we should execute THIS process and then EXIT
          if (lambda_inv_select < lambda_inv_sum) {
            A_.doInteraction(view);
            return EProcessReturn::eInteracted;
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
            return EProcessReturn::eInteracted;
          }
        } // end branch B_
        break;
      }
    }
    return EProcessReturn::eOk;
  }

  template <typename TProcess1, typename TProcess2, typename TSelect>
  template <typename TParticle>
  inline InverseTimeType
  SwitchProcessSequence<TProcess1, TProcess2, TSelect>::getInverseLifetime(
      TParticle&& particle) {

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

  template <typename TProcess1, typename TProcess2, typename TSelect>
  // select decay process
  template <typename TSecondaryView>
  inline EProcessReturn SwitchProcessSequence<TProcess1, TProcess2, TSelect>::selectDecay(
      TSecondaryView& view, [[maybe_unused]] InverseTimeType decay_inv_select,
      [[maybe_unused]] InverseTimeType decay_inv_sum) {
    switch (select_(view.parent())) {
      case SwitchResult::First: {
        if constexpr (t1ProcSeq) {
          // if A_ is a process sequence --> check inside
          EProcessReturn const ret =
              A_.selectDecay(view, decay_inv_select, decay_inv_sum);
          // if A_ did succeed, stop routine here (not checking other static branch B_)
          if (ret != EProcessReturn::eOk) { return ret; }
        } else if constexpr (std::is_base_of_v<DecayProcess<process1_type>,
                                               process1_type>) {
          // if this is not a ContinuousProcess --> evaluate probability
          decay_inv_sum += A_.getInverseLifetime(view.parent());
          // check if we should execute THIS process and then EXIT
          if (decay_inv_select < decay_inv_sum) {
            // more pedagogical: rndm_select < decay_inv_sum / decay_inv_tot
            A_.doDecay(view);
            return EProcessReturn::eDecayed;
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
            return EProcessReturn::eDecayed;
          }
        } // end branch B_
        break;
      }
    }
    return EProcessReturn::eOk;
  }

  /// traits marker to identify objectas ProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_process_sequence<SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

  /// traits marker to identify objectas SwitchProcessSequence
  template <typename TProcess1, typename TProcess2, typename TSelect>
  struct is_switch_process_sequence<SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
      : std::true_type {};

} // namespace corsika
