/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ProcessTraits.hpp>

#include <type_traits>

namespace corsika {

  class TDerived; // fwd decl

  /**
     @ingroup Processes
     @{

     Each process in C8 must derive from BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess

     \todo rename BaseProcess into just Process
     \todo rename _BaseProcess, or find better alternative in FIXME
     ./Processes/AnalyticProcessors/ExecTime.h, see e.g. how this is done in
     ProcessSequence.hpp/make_sequence
   */
  class _BaseProcess {};

  template <typename TDerived>
  struct BaseProcess : _BaseProcess {
  protected:
    friend TDerived;

    BaseProcess() = default; // protected constructor will allow only
                             // derived classes to be created, not
                             // BaseProcess itself

    TDerived& ref() { return static_cast<TDerived&>(*this); }
    const TDerived& ref() const { return static_cast<const TDerived&>(*this); }

  public:
    //! Default number of processes ist just one, obviously
    static unsigned int constexpr getNumberOfProcesses() { return 1; }

    // Base processor type for use in other template classes
    using process_type = TDerived;
  };

  /**
   * ProcessTraits specialization
   **/
  template <typename TProcess>
  struct is_process<
      TProcess,
      std::enable_if_t<std::is_base_of_v<BaseProcess<typename std::decay_t<TProcess>>,
                                         typename std::decay_t<TProcess>>>>
      : std::true_type {};

  template <typename TProcess, int N>
  struct count_processes<TProcess, N,
                         typename std::enable_if_t<is_process_v<TProcess> &&
                                                   !is_process_sequence_v<TProcess>>> {
    static unsigned int constexpr count = N + 1;
  };

  //! @}
  
} // namespace corsika
