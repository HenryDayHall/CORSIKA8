/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_stackprocess_h_
#define _include_corsika_stackprocess_h_

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

  template <typename derived>
  class StackProcess {

  public:
    StackProcess() = delete;
    StackProcess(const unsigned int nStep)
        : fNStep(nStep) {}

    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }

    /// here starts the interface-definition part
    // -> enforce derived to implement DoStack...
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

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const StackProcess<T>* impl);

} // namespace corsika::process

#endif
