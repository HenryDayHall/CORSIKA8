/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/process/ProcessReturn.h> // for convenience
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

  /**
     \class SecondariesProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type SecondariesProcess<T>

   */

  template <typename TDerived>
  class SecondariesProcess : public BaseProcess<TDerived> {
    public:
    using _TDerived = TDerived;
   
    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoSecondaries...
    template <typename TSecondaries>
    inline EProcessReturn DoSecondaries(TSecondaries&);
  };

} // namespace corsika::process
