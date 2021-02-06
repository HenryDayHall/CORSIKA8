/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  /**
     Process that modifies a list of secondaries of other processes

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
    void doSecondaries(TSecondaries&);
  };

} // namespace corsika
