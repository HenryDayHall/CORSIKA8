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
  template <typename TProcess, typename Enable = void>
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
   *  A traits marker to track which BaseProcess is also a ProcessSequence
   **/
  template <typename TClass>
  struct is_process_sequence : std::false_type {};

  template <typename TClass>
  bool constexpr is_process_sequence_v = is_process_sequence<TClass>::value;

  /**
   * A traits marker to identiy a BaseProcess that is also SwitchProcessesSequence
   **/

  template <typename TClass>
  struct is_switch_process_sequence : std::false_type {};

  template <typename TClass>
  bool constexpr is_switch_process_sequence_v = is_switch_process_sequence<TClass>::value;

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
