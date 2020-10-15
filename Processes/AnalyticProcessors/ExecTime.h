/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <chrono>
#include <type_traits>

#include <corsika/logging/Logging.h>

#include <corsika/process/BoundaryCrossingProcess.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/process/StackProcess.h>

#include <corsika/process/analytic_processors/ImplBoundary.h>
#include <corsika/process/analytic_processors/ImplContinuous.h>
#include <corsika/process/analytic_processors/ImplDecay.h>
#include <corsika/process/analytic_processors/ImplInteraction.h>
#include <corsika/process/analytic_processors/ImplSecondaries.h>

namespace corsika::process {
  namespace analytic_processors {

    ///  Time measurement of individual processes
    /** This class allowes to log the runtime spend in all default calls to the process. 
     *  No distinction is made between individual function calls of the process, the runtime is accumulated 
     *  and or avaraged without differentiation.
     *  
     *  The class is currently only implemented for BoundaryProcess, ContinuousProcess, DecayProcess, InteractionProcess and SecondariesProcess and captures only the according functions of the base class given as template parameter. Trying to access BoundaryProcess functions with a DecayProcess as template parameter will currently give long errormessages. 
     * 
     * Inherits all functionality of the class that should be measured, this includes functions like getters and setters
     */
    template <typename T>
    class ExecTime
        : public Boundary<T, std::is_base_of<corsika::process::BoundaryCrossingProcess<
                                                 typename T::TProcessType>,
                                             T>::value>,
          public Continuous<
              T,
              std::is_base_of<corsika::process::ContinuousProcess<typename T::TProcessType>,
                              T>::value>,
          public Decay<
              T, std::is_base_of<corsika::process::DecayProcess<typename T::TProcessType>,
                                 T>::value>,
          public Interaction<
              T,
              std::is_base_of<corsika::process::InteractionProcess<typename T::TProcessType>,
                              T>::value>,
          public Secondaries<
              T,
              std::is_base_of<corsika::process::SecondariesProcess<typename T::TProcessType>,
                              T>::value> {
      static_assert(std::is_base_of<corsika::process::_BaseProcess, T>::value,
                    "error message");

    public:
      ~ExecTime() {
        C8LOG_INFO("Accumulated time spend in process {} is {} µs", typeid(T).name(),
                   this->sumTime());
      }
    };
  } // namespace analytic_processors
} // namespace corsika::process