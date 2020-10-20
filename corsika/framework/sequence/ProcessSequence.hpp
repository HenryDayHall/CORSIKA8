/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cmath>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/sequence/BaseProcess.hpp>
#include <corsika/framework/sequence/BoundaryCrossingProcess.hpp>
#include <corsika/framework/sequence/ContinuousProcess.hpp>
#include <corsika/framework/sequence/DecayProcess.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>
#include <corsika/framework/sequence/ProcessReturn.hpp>
#include <corsika/framework/sequence/SecondariesProcess.hpp>
#include <corsika/framework/sequence/StackProcess.hpp>
#include <limits>
#include <type_traits>

namespace corsika {

  /**
   * FIXME
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

    // example for a trait-based call:
    // void Hello() const  { detail::CallHello<T1,T2>::Call(A, B); }

    template <typename Particle, typename VTNType>
    EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                      VTNType const& to);

    template <typename TParticle, typename TTrack>
    EProcessReturn DoContinuous(TParticle& vP, TTrack& vT);

    template <typename TSecondaries>
    EProcessReturn DoSecondaries(TSecondaries& vS);

    /**
       The processes of type StackProcess do have an internal counter,
       so they can be exectuted only each N steps. Often these are
       "maintenacne processes" that do not need to run after each
       single step of the simulations. In the CheckStep function it is
       tested if either A_ or B_ are StackProcess and if they are due
       for execution.
     */
    bool CheckStep();

    /**
       Execute the StackProcess-es in the ProcessSequence
     */
    template <typename TStack>
    EProcessReturn DoStack(TStack& vS);

    template <typename TParticle, typename TTrack>
    corsika::units::si::LengthType MaxStepLength(TParticle& vP, TTrack& vTrack);
    template <typename TParticle>
    corsika::units::si::GrammageType GetTotalInteractionLength(TParticle& vP);

    template <typename TParticle>
    inline corsika::units::si::InverseGrammageType GetTotalInverseInteractionLength(
        TParticle& vP);

    template <typename TParticle>
    inline corsika::units::si::InverseGrammageType GetInverseInteractionLength(
        TParticle& vP);

    template <typename TParticle, typename TSecondaries>
    EProcessReturn SelectInteraction(
        TParticle& vP, TSecondaries& vS,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_select,
        corsika::units::si::InverseGrammageType& lambda_inv_count);

    template <typename TParticle>
    corsika::units::si::TimeType GetTotalLifetime(TParticle& p);

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetTotalInverseLifetime(TParticle& p);

    template <typename TParticle>
    corsika::units::si::InverseTimeType GetInverseLifetime(TParticle& p);

    // select decay process
    template <typename TParticle, typename TSecondaries>
    EProcessReturn SelectDecay(
        TParticle& vP, TSecondaries& vS,
        [[maybe_unused]] corsika::units::si::InverseTimeType decay_select,
        corsika::units::si::InverseTimeType& decay_inv_count);

    void Init() {
      A.Init();
      B.Init();
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

  /// marker to identify objectas ProcessSequence
  template <typename A, typename B>
  struct is_process_sequence<corsika::ProcessSequence<A, B>> : std::true_type {};

} // namespace corsika

#include <corsika/detail/framework/sequence/ProcessSequence.inl>
