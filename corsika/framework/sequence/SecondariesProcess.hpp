/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/sequence/ProcessReturn.hpp> // for convenience
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {

  /**
     \class SecondariesProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type SecondariesProcess<T>

   */

  template <typename TDerived>
  class SecondariesProcess : public BaseProcess<TDerived> {
  public:
    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoSecondaries...
    template <typename TSecondaries>
    inline void DoSecondaries(TSecondaries&);
  };

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const SecondariesProcess<T>* impl);

} // namespace corsika
