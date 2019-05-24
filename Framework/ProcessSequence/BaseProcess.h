/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_baseprocess_h_
#define _include_corsika_baseprocess_h_

#include <corsika/process/ProcessReturn.h> // for convenience
#include <type_traits>

namespace corsika::process {

  /**
     \class BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess<T>

   */

  template <typename Derived>
  struct BaseProcess {
  private:
    BaseProcess() {}
    friend Derived;

  public:
    Derived& GetRef() { return static_cast<Derived&>(*this); }
    const Derived& GetRef() const { return static_cast<const Derived&>(*this); }
  };

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const BaseProcess<T>* impl);

} // namespace corsika::process

#endif
