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
     BaseProcess
   */

  class NullModel { // : public BaseProcess<NullModel> {

  public:
    NullModel() = default;
    ~NullModel() = default;

    //! Default number of processes is just one, obviously
    static unsigned int constexpr getNumberOfProcesses() { return 0; }
  };

  /**
     is_process traits specialization to indicate compatibility ProcessSequence
  */
  template <>
  struct is_process<NullModel, void> : std::true_type {};

  /**
     count_processes traits specialization to increase process count by one.
   */
  template <int N>
  struct count_processes<NullModel, N, void> {
    static unsigned int constexpr count = N;
  };

  //! @}

} // namespace corsika
