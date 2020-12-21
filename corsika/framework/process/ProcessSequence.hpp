n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
 * \file ProcessSequence.hpp
 */

#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/process/ProcessTraits.hpp>
#include <corsika/framework/process/BoundaryCrossingProcess.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/framework/process/DecayProcess.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/framework/process/StackProcess.hpp>
#include <corsika/framework/process/NullModel.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  /**
   *
   *  Definition of a static process list/sequence
   *
   *  A compile time static list of processes. The compiler will
   *  generate a new type based on template logic containing all the
   *  elements provided by the user.
   *
   *  TProcess1 and TProcess2 must both be derived from BaseProcess,
   *  and are both references if possible (lvalue), otherwise (rvalue)
   *  they are just classes. This allows us to handle both, rvalue as
   *  well as lvalue Processes in the ProcessSequence.
   *
   *  The sequence, and the processes use CRTP.
   *
   *  \todo There are several FIXME's in the ProcessSequence.inl due to
   *  outstanding migration of SecondaryView::parent()
   **/

  template <typename TProcess1, typename TProcess2 = NullModel>
  class ProcessSequence : public BaseProcess<ProcessSequence<TProcess1, TProcess2>> {

    using process1_type = typename std::decay_t<TProcess1>;
    using process2_type = typename std::decay_t<TProcess2>;

    static bool constexpr t1ProcSeq = is_process_sequence_v<process1_type>;
    static bool constexpr t2ProcSeq = is_process_sequence_v<process2_type>;

    static bool constexpr t1SwitchProcSeq = is_switch_process_sequence_v<process1_type>;
    static bool constexpr t2SwitchProcSeq = is_switch_process_sequence_v<process2_type>;

    // make sure only BaseProcess types TProcess1/2 are passed
    static_assert(std::is_base_of_v<BaseProcess<process1_type>, process1_type>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 1");
    static_assert(std::is_base_of_v<BaseProcess<process2_type>, process2_type>,
                  "can only use process derived from BaseProcess in "
                  "ProcessSequence, for Process 2");

    TProcess1 A_; /// process/list A, this is a reference, if possible
    TProcess2 B_; /// process/list B, this is a reference, if possible

  public:
    // resource management
    ProcessSequence() = delete; // only initialized objects
    ProcessSequence(ProcessSequence const&) = default;
    ProcessSequence(ProcessSequence&&) = default;
    ProcessSequence& operator=(ProcessSequence const&) = default;
    ~ProcessSequence() = default;

    /**
     * Only valid user constructor will create fully initialized object
     *
     * ProcessSequence supports and encourages move semantics. You can
     * use object, l-value references or r-value references to
     * construct sequences.
     *
     * \param in_A process/list A
     * \param in_A process/list B
     **/
    ProcessSequence(TProcess1 in_A, TProcess2 in_B)
        : A_(in_A)
        , B_(in_B) {}

    template <typename TParticle>
    ProcessReturn doBoundaryCrossing(TParticle& particle,
                                     typename TParticle::node_type const& from,
                                     typename TParticle::node_type const& to);

    template <typename TParticle, typename TTrack>
    inline ProcessReturn doContinuous(TParticle& particle, TTrack& vT);

    template <typename TSecondaries>
    inline void doSecondaries(TSecondaries& vS);

    /**
       The processes of type StackProcess do have an internal counter,
       so they can be exectuted only each N steps. Often these are
       "maintenacne processes" that do not need to run after each
       single step of the simulations. In the CheckStep function it is
       tested if either A_ or B_ are StackProcess and if they are due
       for execution.
     */
    inline bool checkStep();

    /**
       Execute the StackProcess-es in the ProcessSequence
     */
    template <typename TStack>
    inline void doStack(TStack& stack);

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
    inline TimeType getLifetime(TParticle& particle) {
      return 1. / getInverseLifetime(particle);
    }

    template <typename TParticle>
    inline InverseTimeType getInverseLifetime(TParticle&& particle);

    // select decay process
    template <typename TSecondaryView>
    inline ProcessReturn selectDecay(
        TSecondaryView& view, [[maybe_unused]] InverseTimeType decay_inv_select,
        [[maybe_unused]] InverseTimeType decay_inv_sum = InverseTimeType::zero());
  };

  /**
   * Factory function to create ProcessSequence
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
   *
   * \param vA needs to derive from BaseProcess or ProcessSequence
   * \param vB paramter-pack, needs to derive BaseProcess or ProcessSequence
   **/

  template <typename... TProcesses, typename TProcess1>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                        typename std::decay_t<TProcess1>>,
      ProcessSequence<TProcess1, decltype(make_sequence(std::declval<TProcesses>()...))>>
  make_sequence(TProcess1&& vA, TProcesses&&... vBs) {
    return ProcessSequence<TProcess1,
                           decltype(make_sequence(std::declval<TProcesses>()...))>(
        vA, make_sequence(std::forward<TProcesses>(vBs)...));
  }

  /**
   * Factory function to create ProcessSequence
   *
   * specialization for two input objects (no paramter pack in vB).
   *
   * \param vA needs to derive from BaseProcess or ProcessSequence
   * \param vB needs to derive BaseProcess or ProcessSequence
   **/
  template <typename TProcess1, typename TProcess2>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess1>>,
                        typename std::decay_t<TProcess1>> &&
          std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess2>>,
                            typename std::decay_t<TProcess2>>,
      ProcessSequence<TProcess1, TProcess2>>
  make_sequence(TProcess1&& vA, TProcess2&& vB) {
    return ProcessSequence<TProcess1, TProcess2>(vA, vB);
  }

  /**
   * Factory function to create ProcessSequence from a single BaseProcess
   *
   * also allow a single Process in ProcessSequence, accompany by
   * `NullModel`
   *
   * \param vA needs to derive from BaseProcess or ProcessSequence
   **/
  template <typename TProcess>
  inline typename std::enable_if_t<
      std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess>>,
                        typename std::decay_t<TProcess>>,
      ProcessSequence<TProcess, NullModel>>
  make_sequence(TProcess&& vA) {
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

} // namespace corsika

#include <corsika/detail/framework/process/ProcessSequence.inl>
