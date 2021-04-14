/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

//#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/process/ProcessTraits.hpp>

namespace corsika {

  /**
     @ingroup Processes
     @{

     Process that does nothing. It is not even derived from
     BaseProcess. But it can be added to a ProcessSequence.
   */

  class NullModel {

  public:
    NullModel() = default;
    ~NullModel() = default;

    static bool const is_process_sequence = false;
    static bool const is_switch_process_sequence = false;

    //! Default number of processes is zero, obviously
    static unsigned int constexpr getNumberOfProcesses() { return 0; }
  };

  /**
     is_process traits specialization to indicate compatibility with BaseProcess
  */
  template <typename TNull>
  struct is_process<
      TNull, std::enable_if_t<std::is_base_of_v<NullModel, typename std::decay_t<TNull>>>>
      : std::true_type {};

  /**
     count_processes traits specialization to increase process count by one.
   */
  template <int N>
  struct count_processes<NullModel, N, void> {
    static unsigned int constexpr count = N;
  };

  //! @}

} // namespace corsika
