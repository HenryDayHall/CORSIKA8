/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

// the basic particle data stack:
#include <corsika/stack/super_stupid/SuperStupidStack.h>

// extension with nuclear data for Code::Nucleus
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

// extension with nuclear data AND volume node ref
#include <corsika/setup/GeometryNodeStackExtension.h>

#include <memory>
#include <tuple>
#include <utility>

class Event {};

namespace corsika::history {

  /**
   * @class HistoryData
   *
   * definition of stack-data object to store history information
   * this is vector with shared_ptr<Event>
   */
  class HistoryData {

  public:
    // these functions are needed for the Stack interface
    void Clear() { fEvent.clear(); }
    unsigned int GetSize() const { return fEvent.size(); }
    unsigned int GetCapacity() const { return fEvent.size(); }
    void Copy(const int i1, const int i2) { fEvent[i2] = fEvent[i1]; }
    void Swap(const int i1, const int i2) { std::swap(fEvent[i1], fEvent[i2]); }

    // custom data access function
    void SetEvent(const int i, std::shared_ptr<Event> v) { fEvent[i] = v; }
    std::shared_ptr<Event> GetEvent(const int i) const { return fEvent[i]; }

    // these functions are also needed by the Stack interface
    void IncrementSize() { fEvent.push_back(nullptr); }
    void DecrementSize() {
      if (fEvent.size() > 0) { fEvent.pop_back(); }
    }

    // custom private data section
  private:
    std::vector<std::shared_ptr<Event>> fEvent;
  };

  /**
   * @class HistoryDataInterface
   *
   * corresponding defintion of a stack-readout object, the iteractor
   * dereference operator will deliver access to these function
  // defintion of a stack-readout object, the iteractor dereference
  // operator will deliver access to these function
   */
  template <typename T>
  class HistoryDataInterface : public T {

  public:
    using T::GetIndex;
    using T::GetStackData;
    using T::SetParticleData;

    // default version for particle-creation from input data
    void SetParticleData(const std::tuple<Event const*> v) { SetEvent(std::get<0>(v)); }
    void SetParticleData(HistoryDataInterface& parent,
                         const std::tuple<std::shared_ptr<Event>>) {
      SetEvent(parent.GetEvent()); // copy Event from parent particle!
    }
    void SetParticleData() { SetEvent(nullptr); }
    void SetParticleData(HistoryDataInterface& parent) {
      SetEvent(parent.GetEvent()); // copy Event from parent particle!
    }
    void SetEvent(std::shared_ptr<Event> v) { GetStackData().SetEvent(GetIndex(), v); }
    std::shared_ptr<Event> GetEvent() const {
      return GetStackData().GetEvent(GetIndex());
    }
  };

  namespace detail {

    //
    // this is an auxiliary help typedef, which I don't know how to put
    // into NuclearStackExtension.h where it belongs...
    template <typename StackIter>
    using ExtendedParticleInterfaceType =
        corsika::stack::nuclear_extension::NuclearParticleInterface<
            corsika::stack::super_stupid::SuperStupidStack::PIType, StackIter>;
    //

    // the particle data stack with extra nuclear information:
    using ParticleDataStack = corsika::stack::nuclear_extension::NuclearStackExtension<
        corsika::stack::super_stupid::SuperStupidStack, ExtendedParticleInterfaceType>;

    template <typename T>
    using SetupHistoryDataInterface = HistoryDataInterface<T>;

    // combine particle data stack with history information for tracking
    template <typename StackIter>
    using StackWithHistoryInterface =
        corsika::stack::CombinedParticleInterface<ParticleDataStack::PIType,
                                                  SetupHistoryDataInterface, StackIter>;

    using StackWithHistory =
        corsika::stack::CombinedStack<typename ParticleDataStack::StackImpl, HistoryData,
                                      StackWithHistoryInterface>;

  } // namespace detail

  template <typename InnerStack, template <typename> typename _PI>
  using NuclearStackExtension =
      Stack<NuclearStackExtensionImpl<typename InnerStack::StackImpl>, _PI>;

} // namespace corsika::history
