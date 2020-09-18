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
  class HSecondaryView : public TView {
    
    std::shared_ptr<Event> event_;
    
    using StackIterator = typename TView::StackIteratorValue;
    
  public:
    HSecondaryView(StackIterator& p)
      : TView(p),
	event_{std::make_shared<Event>(p.GetIndex())} {
      p.SetEvent(event_); // here an entry on the main particle stack obtains its Event
                          // RU: what seems to missing to me right now, at 2am..., is the
                          // actual back reference to the parent event. This needs to be added.
    }

    template <typename... Args>
    void AddSecondary(Args&&... args) {
      auto s = TView::AddSecondary(
          std::forward<Args...>(args...));
      // store particles at production time here
      // RU: consider if we can call
      // TView::AddSecondary twice instead: 1. for particle at production time
      // , 2. dynamic particle ... not sure, but would be extremely flexible. 
      event_->addSecondary(s.GetEnergy(), s.GetMomentum(), s.GetPID());
    }

    // it is probably better to have one method "GetEvent()" and then one can always call GetEvent().set/getWhatever(...) 
    void SetTarget(const particles::Code targetCode) { event_->setTargetCode(targetCode); }
  };

} // namespace corsika::history
