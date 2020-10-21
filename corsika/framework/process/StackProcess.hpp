/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ProcessReturn.hpp> // for convenience

namespace corsika {

  /**
     \class StackProcess

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
        : fNStep(nStep) {}

    /// here starts the interface-definition part
    // -> enforce TDerived to implement DoStack...
    template <typename TStack>
    inline void DoStack(TStack&);

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

} // namespace corsika
