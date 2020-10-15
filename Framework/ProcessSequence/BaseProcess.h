/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/ProcessReturn.h> // for convenience

namespace corsika::process {

  class TDerived; // fwd decl

  /**
     \class BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess<T>

   */
  class _BaseProcess{};

  template <typename TDerived>
  class BaseProcess : _BaseProcess {
  protected:
    friend TDerived;

    BaseProcess() = default; // protected constructor will allow only
                             // derived classes to be created, not
                             // BaseProcess itself

    TDerived& GetRef() { return static_cast<TDerived&>(*this); }
    const TDerived& GetRef() const { return static_cast<const TDerived&>(*this); }

    public:
      // Base processor type for use in other template classes
      using TProcessType = TDerived;
  };

} // namespace corsika::process
