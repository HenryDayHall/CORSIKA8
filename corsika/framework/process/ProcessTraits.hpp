/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
 * @file ProcessTraits.hpp
 */

#include <type_traits>

namespace corsika {

  /**
   * A traits marker to identify BaseProcess, thus any type of process
   */
  template <typename TProcess, typename TEnable = void>
  struct is_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_process_v = is_process<TProcess>::value;

  /**
   * A traits marker to identify ContinuousProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_continuous_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_continuous_process_v = is_continuous_process<TProcess>::value;

  /**
   * A traits marker to identify DecayProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_decay_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_decay_process_v = is_decay_process<TProcess>::value;

  /**
   * A traits marker to identify StackProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_stack_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_stack_process_v = is_stack_process<TProcess>::value;

  /**
   * A traits marker to identify SecondariesProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_secondaries_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_secondaries_process_v = is_secondaries_process<TProcess>::value;

  /**
   * A traits marker to identify BoundaryProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_boundary_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_boundary_process_v = is_boundary_process<TProcess>::value;

  /**
   * A traits marker to identify InteractionProcess
   */
  template <typename TProcess, typename Enable = void>
  struct is_interaction_process : std::false_type {};

  template <typename TProcess>
  bool constexpr is_interaction_process_v = is_interaction_process<TProcess>::value;

  /**
   * A traits marker to identify ProcessSequence that contain a StackProcess
   **/
  template <typename TClass>
  struct contains_stack_process : std::false_type {};

  template <typename TClass>
  bool constexpr contains_stack_process_v = contains_stack_process<TClass>::value;

  /**
   * traits class to count any type of Process, general version
   **/
  template <typename TProcess, int N = 0, typename Enable = void>
  struct count_processes {
    static unsigned int constexpr count = N;
  };

  namespace detail {

    /**
       Helper traits class (partial) for static compile time checking.

       Note, this is a poor replacement for C++20 concepts... they are
       eagerly awaited!

       It defines the default body of a generic test function returning
       std::false_type.

       In addition it defines the pattern for class-method matching with a
       return type TReturn and function arguments TArgs... . Right now
       both method signatures, "const" and "not const", are matched.
     */
    template <typename TReturn, typename... TArgs>
    struct has_method_signature {

      // the non-const version
      template <class T>
      static std::true_type testSignature(TReturn (T::*)(TArgs...));

      // the const version
      template <class T>
      static std::true_type testSignature(TReturn (T::*)(TArgs...) const);
    };

  } // namespace detail

} // namespace corsika
