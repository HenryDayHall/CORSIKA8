/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/Stack.h>
#include <corsika/logging/Logging.h>

#include <memory>
#include <utility>
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
    using EventPtr =
        std::shared_ptr<TEvent>; //!< Pointer to the event where this particle was created
    using ParentEventIndex = int; //!< index to TEvent::secondaries_
    using DataType = std::pair<EventPtr, ParentEventIndex>;

  public:
    // these functions are needed for the Stack interface
    void Clear() { historyData_.clear(); }
    unsigned int GetSize() const { return historyData_.size(); }
    unsigned int GetCapacity() const { return historyData_.size(); }
    void Copy(const int i1, const int i2) { historyData_[i2] = historyData_[i1]; }
    void Swap(const int i1, const int i2) {
      std::swap(historyData_[i1], historyData_[i2]);
    }

    // custom data access function
    void SetEvent(const int i, EventPtr v) { historyData_[i].first = std::move(v); }
    EventPtr GetEvent(const int i) const { return historyData_[i].first; }

    void SetParentEventIndex(const int i, ParentEventIndex v) {
      historyData_[i].second = std::move(v);
    }
    ParentEventIndex GetParentEventIndex(const int i) const {
      return historyData_[i].second;
    }

    // these functions are also needed by the Stack interface
    void IncrementSize() { historyData_.push_back(DataType{}); }
    void DecrementSize() {
      if (historyData_.size() > 0) { historyData_.pop_back(); }
    }

    // custom private data section
  private:
    std::vector<DataType> historyData_;
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
    void SetParticleData() {
      C8LOG_TRACE("HistoyDatatInterface::SetParticleData()");
      GetStackData().SetParentEventIndex(GetIndex(), -1); }

    // create a new particle as secondary of a parent
    void SetParticleData(HistoryDataInterface& /*parent*/) {
            C8LOG_TRACE("HistoyDatatInterface::SetParticleData(parnt)");
	    SetParticleData();


      // store particles at production time in Event here
      auto const sec_index = event_->addSecondary(
          stack_sec.GetEnergy(), stack_sec.GetMomentum(), stack_sec.GetPID());
      stack_sec.SetParentEventIndex(sec_index);
      stack_sec.SetEvent(event_);
    }

    void SetEvent(const std::shared_ptr<TEvent>& v) {
      GetStackData().SetEvent(GetIndex(), v);
    }

    void SetParentEventIndex(int index) {
      GetStackData().SetParentEventIndex(GetIndex(), index);
    }

    std::shared_ptr<TEvent> GetEvent() const {
      return GetStackData().GetEvent(GetIndex());
    }

    int GetParentEventIndex() const {
      return GetStackData().GetParentEventIndex(GetIndex());
    }
  };

  template <typename T, typename TEvent>
  struct MakeHistoryDataInterface {
    typedef HistoryDataInterface<T, TEvent> type;
  };

} // namespace corsika::history

// for user-friendlyness we create the HistoryDataInterface type
// with the histoy::Event data content right here:

#include <corsika/history/Event.hpp>

namespace corsika::history {

  template <typename TStackIter>
  using HistoryEventDataInterface =
      typename history::MakeHistoryDataInterface<TStackIter, history::Event>::type;

  using HistoryEventData = history::HistoryData<history::Event>;

} // namespace corsika::history
