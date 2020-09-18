/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/Stack.h>
#include <corsika/history/Event.hpp>

#include <memory>
#include <tuple>
#include <vector>

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
    void Clear() { event_.clear(); }
    unsigned int GetSize() const { return event_.size(); }
    unsigned int GetCapacity() const { return event_.size(); }
    void Copy(const int i1, const int i2) { event_[i2] = event_[i1]; }
    void Swap(const int i1, const int i2) { std::swap(event_[i1], event_[i2]); }

    // custom data access function
    void SetEvent(const int i, std::shared_ptr<TEvent> v) { event_[i] = std::move(v); }
    std::shared_ptr<TEvent> GetEvent(const int i) const { return event_[i]; }

    // these functions are also needed by the Stack interface
    void IncrementSize() { event_.push_back(nullptr); }
    void DecrementSize() {
      if (event_.size() > 0) { event_.pop_back(); }
    }

    // custom private data section
  private:
    std::vector<std::shared_ptr<TEvent>> event_;
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
    // create a new particle from scratch
    void SetParticleData() {} // nullptr, already by design

    // create a new particle as secondary of a parent
    void SetParticleData(HistoryDataInterface& parent) { SetParticleData(); }

    void SetEvent(const std::shared_ptr<TEvent>& v) {
      GetStackData().SetEvent(GetIndex(), v);
    }

    std::shared_ptr<TEvent> GetEvent() const {
      return GetStackData().GetEvent(GetIndex());
    }
  };

  template <typename T, typename TEvent>
  struct MakeHistoryDataInterface {
    typedef HistoryDataInterface<T, TEvent> type;
  };

} // namespace corsika::history
