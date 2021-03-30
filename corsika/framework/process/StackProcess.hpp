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
     @ingroup Processes
     @{

     Process to act on the entire particle stack

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type StackProcess<T>

   */

  template <typename TDerived>
  class StackProcess : public BaseProcess<TDerived> {
  private:
  protected:
  public:
    StackProcess() = delete;
    StackProcess(const unsigned int nStep)
        : nStep_(nStep) {}

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoStack...
    template <typename TStack>
    void doStack(TStack&);

    int getStep() const { return iStep_; }
    bool checkStep() { return !((++iStep_) % nStep_); }

  private:
    /**
       @name The number of "steps" during the cascade processing after
       which this StackProcess is going to be executed. The logic is
       "fIStep modulo fNStep"
       @{
     */
    unsigned int nStep_ = 0;
    unsigned long int iStep_ = 0;
    //! @}
  };

  //! @}
  
} // namespace corsika
