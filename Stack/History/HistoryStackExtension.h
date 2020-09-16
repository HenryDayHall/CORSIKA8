/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/Stack.h>

#include <tuple>
#include <vector>
#include <memory>

namespace corsika::history {

  /**
   * @class HistoryData
   *
   * definition of stack-data object to store history information this
   * is vector with shared_ptr<TEvent>, where TEvent is a free
   * template parameter for customization.
   */
  template <typename TEvent>
  class HistoryData {

  public:
    // these functions are needed for the Stack interface
    void Clear() { fEvent.clear(); }
    unsigned int GetSize() const { return fEvent.size(); }
    unsigned int GetCapacity() const { return fEvent.size(); }
    void Copy(const int i1, const int i2) { fEvent[i2] = fEvent[i1]; }
    void Swap(const int i1, const int i2) { std::swap(fEvent[i1], fEvent[i2]); }

    // custom data access function
    void SetEvent(const int i, std::shared_ptr<TEvent> v) { fEvent[i] = v; }
    std::shared_ptr<TEvent> GetEvent(const int i) const { return fEvent[i]; }

    // these functions are also needed by the Stack interface
    void IncrementSize() { fEvent.push_back(nullptr); }
    void DecrementSize() {
      if (fEvent.size() > 0) { fEvent.pop_back(); }
    }

    // custom private data section
  private:
    std::vector<std::shared_ptr<TEvent>> fEvent;
  };

  /**
   * @class HistoryDataInterface
   *
   * corresponding defintion of a stack-readout object, the iteractor
   * dereference operator will deliver access to these function
  // defintion of a stack-readout object, the iteractor dereference
  // operator will deliver access to these function
   */
  template <typename T, typename TEvent>
  class HistoryDataInterface : public T {
    protected:
      using T::GetStack;
      using T::GetStackData;

    public:
      using T::GetIndex;

  public:
    // default version for particle-creation from input data
    void SetParticleData(const std::tuple<TEvent const*> v) { SetEvent(std::get<0>(v)); }
    void SetParticleData(HistoryDataInterface& parent,
                         const std::tuple<std::shared_ptr<TEvent>>) {
      SetEvent(parent.GetEvent()); // copy Event from parent particle!
    }
    void SetParticleData() { SetEvent(nullptr); }
    void SetParticleData(HistoryDataInterface& parent) {
      SetEvent(parent.GetEvent()); // copy Event from parent particle!
    }
    void SetEvent(std::shared_ptr<TEvent> v) { GetStackData().SetEvent(GetIndex(), v); }
    std::shared_ptr<TEvent> GetEvent() const {
      return GetStackData().GetEvent(GetIndex());
    }
  };
  
  template <typename T, typename TEvent>
    struct MakeHistoryDataInterface {
      typedef HistoryDataInterface<T, TEvent> type;
    };
  
  
} // namespace corsika::history
