/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
 * \file ProcessTraits.hpp
 */

#include <corsika/framework/process/ProcessTraits.hpp>

#include <type_traits>

namespace corsika {

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
   * traits class to count ContinuousProcess-es
   **/
    template <typename TProcess, int N=0, typename Enable=void>
    struct count_continuous {
      enum { count = N };
    }; 


  /**
   * traits class to count ContinuousProcess-es
   **/
    template <typename TProcessSequence, int N>
    struct count_continuous<TProcessSequence, N, 
                           typename std::enable_if_t<is_process_sequence_v<TProcessSequence>>> {
      enum { count = N+TProcessSequence::nContinuous };
    };

  /**
   * traits class to count ContinuousProcess-es
   **/

    template <typename TSwitchProcessSequence, int N>
    struct count_continuous<TSwitchProcessSequence, N, 
                           typename std::enable_if_t<is_switch_process_sequence_v<TSwitchProcessSequence>>> {
      enum { count = N+TSwitchProcessSequence::nContinuous };
    };




} // namespace corsika
