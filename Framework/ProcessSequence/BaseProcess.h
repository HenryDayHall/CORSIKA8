/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/ProcessReturn.h> // for convenience
#include <type_traits>

namespace corsika::process {

  class TDerived; // fwd decl

  /**
     \class BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess<T>

   */

  template <typename TDerived>
  class BaseProcess {
  protected:
    friend TDerived;

    BaseProcess() = default; // protected constructor will allow only
                             // derived classes to be created, not
                             // BaseProcess itself

    TDerived& GetRef() { return static_cast<TDerived&>(*this); }
    const TDerived& GetRef() const { return static_cast<const TDerived&>(*this); }
  };

} // namespace corsika::process
