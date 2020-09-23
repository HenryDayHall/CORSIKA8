/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/SecondaryView.h>
#include <corsika/history/Event.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace corsika::history {

  template <typename TView>
  class HistorySecondaryView : public TView {

    EventPtr event_;

    using StackIteratorValue = typename TView::StackIteratorValue;
    using StackIterator = typename TView::StackIterator;

  public:
    HistorySecondaryView(StackIteratorValue& p)
        : TView{p}
        , event_{std::make_shared<Event>()} {
      event_->setProjectileIndex(p.GetIndex());
      event_->setParentEvent(p.GetEvent());
    }

    template <typename... Args>
    StackIterator AddSecondary(Args&&... args) {
      auto stack_sec = TView::AddSecondary(std::forward<Args...>(args...));

      // store particles at production time in Event here
      auto const sec_index = event_->addSecondary(
          stack_sec.GetEnergy(), stack_sec.GetMomentum(), stack_sec.GetPID());
      stack_sec.SetParentEventIndex(sec_index);
      stack_sec.SetEvent(event_);

      // RU: consider if we can call
      // TView::AddSecondary twice instead: 1. for particle at production time
      // , 2. dynamic particle ... not sure, but would be extremely flexible.

      return stack_sec;
    }
  };

} // namespace corsika::history
