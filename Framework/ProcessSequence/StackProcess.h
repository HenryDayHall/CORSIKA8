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
     \class StackProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type StackProcess<T>

   */

  template <typename TDerived>
  class StackProcess : public BaseProcess<TDerived>{
  private:
  protected:
  public:
    using _TDerived = TDerived;
    
    StackProcess() = delete;
    StackProcess(const unsigned int nStep)
        : fNStep(nStep) {}

<<<<<<< HEAD
=======
    TDerived& GetRef() { return static_cast<TDerived&>(*this); }
    const TDerived& GetRef() const { return static_cast<const TDerived&>(*this); }

>>>>>>> renamed base process templates
    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoStack...
    template <typename TStack>
    inline EProcessReturn DoStack(TStack&);

    int GetStep() const { return fIStep; }
    bool CheckStep() { return !((++fIStep) % fNStep); }

  private:
    /**
       @name The number of "steps" during the cascade processing after
       which this StackProcess is going to be executed. The logic is
       "fIStep modulo fNStep"
       @{
     */
    unsigned int fNStep = 0;
    unsigned long int fIStep = 0;
    //! @}
  };

} // namespace corsika::process
