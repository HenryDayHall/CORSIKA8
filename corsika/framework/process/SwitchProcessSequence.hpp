/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
 * \file SwitchProcessSequence.hpp
 **/

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

  /**
   * enum for the process switch selection: identify if First or
   * Second process branch should be used.
   **/
  enum class SwitchResult { First, Second };

  /**
     Class to switch between two process branches

     A compile-time static list of processes that uses an internal
     TSelect class to switch between different versions of processes
     (or process sequence).

     TProcess1 and TProcess2 must be derived from BaseProcess and are
     both references if possible (lvalue), otherwise (rvalue) they are
     just classes. This allows us to handle both, rvalue as well as
     lvalue Processes in the SwitchProcessSequence.

     TSelect has to implement a `operator()(const Particle&)` and has to
     return either SwitchResult::First or SwitchResult::Second. Note:
     TSelect may absolutely also use random numbers to sample between
     its results. This can be used to achieve arbitrarily smooth
     transition or mixtures of processes.

     Warning: do not put StackProcess into a SwitchProcessSequence
     since this makes no sense. The StackProcess acts on an entire
     particle stack and not on indiviidual particles.

     See also class \sa ProcessSequence
  **/

  template <typename TProcess1, typename TProcess2, typename TSelect>
  class SwitchProcessSequence
      : public BaseProcess<SwitchProcessSequence<TProcess1, TProcess2, TSelect>> {

    using process1_type = typename std::decay_t<TProcess1>;
    using process2_type = typename std::decay_t<TProcess2>;

    static bool constexpr t1ProcSeq = is_process_sequence_v<process1_type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<process2_type>;

    // make sure only BaseProcess types TProcess1/2 are passed
    static_assert(std::is_base_of_v<BaseProcess<process1_type>, process1_type>,
                  "can only use process derived from BaseProcess in "
                  "SwitchProcessSequence, for Process 1");
    static_assert(std::is_base_of_v<BaseProcess<process2_type>, process2_type>,
                  "can only use process derived from BaseProcess in "
                  "SwitchProcessSequence, for Process 2");

    // make sure none of TProcess1/2 is a StackProcess
    static_assert(!std::is_base_of_v<StackProcess<process1_type>, process1_type>,
                  "cannot use StackProcess in SwitchProcessSequence, for Process 1");
    static_assert(!std::is_base_of_v<StackProcess<process2_type>, process2_type>,
                  "cannot use StackProcess in SwitchProcessSequence, for Process 2");

    // if TProcess1/2 are already ProcessSequences, make sure they do not contain
    // any StackProcess
    static_assert(!contains_stack_process_v<process1_type>,
                  "cannot use StackProcess in SwitchProcessSequence, remove from "
                  "ProcessSequence 1");
    static_assert(!contains_stack_process_v<process2_type>,
                  "cannot use StackProcess in SwitchProcessSequence, remove from "
                  "ProcessSequence 2");

  public:
    // resource management
    SwitchProcessSequence() = delete; // only initialized objects
    SwitchProcessSequence(SwitchProcessSequence const&) = default;
    SwitchProcessSequence(SwitchProcessSequence&&) = default;
    SwitchProcessSequence& operator=(SwitchProcessSequence const&) = default;
    ~SwitchProcessSequence() = default;

    /**
     * Only valid user constructor will create fully initialized object
     *
     * SwitchProcessSequence supports and encourages move semantics. You can
     * use object, l-value references or r-value references to
     * construct sequences.
     *
     * \param in_A process branch A
     * \param in_A process branch B
     * \param sel functor to swtich between branch A and B
     **/
    SwitchProcessSequence(TProcess1 in_A, TProcess2 in_B, TSelect sel)
        : select_(sel)
        , A_(in_A)
        , B_(in_B) {}

    template <typename TParticle, typename TVTNType>
    ProcessReturn doBoundaryCrossing(TParticle& particle, TVTNType const& from,
                                     TVTNType const& to);

    template <typename TParticle, typename TTrack>
    inline ProcessReturn doContinuous(TParticle& particle, TTrack& vT);

    template <typename TSecondaries>
    inline void doSecondaries(TSecondaries& vS);

    template <typename TParticle, typename TTrack>
    inline LengthType getMaxStepLength(TParticle& particle, TTrack& vTrack);

    template <typename TParticle>
    inline GrammageType getInteractionLength(TParticle&& particle) {
      return 1. / getInverseInteractionLength(particle);
    }

    template <typename TParticle>
    inline InverseGrammageType getInverseInteractionLength(TParticle&& particle);

    template <typename TSecondaryView>
    inline ProcessReturn selectInteraction(
        TSecondaryView& view, [[maybe_unused]] InverseGrammageType lambda_inv_select,
        [[maybe_unused]] InverseGrammageType lambda_inv_sum =
            InverseGrammageType::zero());

    template <typename TParticle>
    inline TimeType getLifetime(TParticle&& particle) {
      return 1. / getInverseLifetime(particle);
    }

    template <typename TParticle>
    inline InverseTimeType getInverseLifetime(TParticle&& particle);

    // select decay process
    template <typename TSecondaryView>
    inline ProcessReturn selectDecay(
        TSecondaryView& view, [[maybe_unused]] InverseTimeType decay_inv_select,
        [[maybe_unused]] InverseTimeType decay_inv_sum = InverseTimeType::zero());

  private:
    TSelect select_; /// selector functor to switch between branch a and b, this is a
                     /// reference, if possible

    TProcess1 A_; /// process branch a, this is a reference, if possible
    TProcess2 B_; /// process branch b, this is a reference, if possible
  };

  /**
   *
   * the functin `make_select(proc1,proc1,selector)` assembles many
   * BaseProcesses, and ProcessSequences into a SwitchProcessSequence,
   * all combinatorics must be allowed, this is why we define a macro
   * to define all combinations here:
   *
   *
   * Both, Processes1 and Processes2, must derive from BaseProcesses
   **/

  template <typename TProcess1, typename TProcess2, typename TSelect>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                        typename std::decay_t<TProcess1>> &&
          std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess2>>,
                            typename std::decay_t<TProcess2>>,
      SwitchProcessSequence<TProcess1, TProcess2, TSelect>>
  make_select(TProcess1&& vA, TProcess2&& vB, TSelect selector) {
    return SwitchProcessSequence<TProcess1, TProcess2, TSelect>(vA, vB, selector);
  }

} // namespace corsika

#include <corsika/detail/framework/process/SwitchProcessSequence.inl>
