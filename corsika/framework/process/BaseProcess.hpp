/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

//#include <corsika/framework/process/ProcessTraits.hpp>

#include <type_traits>

namespace corsika {

  class TDerived; // fwd decl

  /**
     Each process in C8 must derive from BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess<T>

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
    // Base processor type for use in other template classes
    using process_type = TDerived;
  };

} // namespace corsika
