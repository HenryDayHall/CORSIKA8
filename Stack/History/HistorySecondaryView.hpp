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

    EvtPtr event_;

    using StackIteratorValue = typename TView::StackIteratorValue;
    using StackIterator = typename TView::StackIterator;

  public:
    HistorySecondaryView(StackIteratorValue& p)
        : TView{p}
        , event_{std::make_shared<Event>()} {
      event_->setProjectileIndex(p.GetIndex());
      event_->

      //	event_{std::make_shared<Event>(p.GetIndex())} {
      // p.SetEvent(event_); // here an entry on the main particle stack obtains its Event
      // RU: what seems to missing to me right now, at 2am..., is the
      // actual back reference to the parent event. This needs to be added.
    }

    template <typename... Args>
    StackIterator AddSecondary(Args&&... args) {
      auto sec = TView::AddSecondary(std::forward<Args...>(args...));
      // generate new Event for all secondaries to link them to
      // anchestor (aka projectile, here).
      /*auto sec_event = std::make_shared<Event>();
      sec_event->setParentEventAndSecondaryIndex(event_, event_->secondaries().size());
      sec.SetEvent(sec_event);*/

      // store particles at production time in parent/projectile Event here
      event_->addSecondary(sec.GetEnergy(), sec.GetMomentum(), sec.GetPID());

      // RU: consider if we can call
      // TView::AddSecondary twice instead: 1. for particle at production time
      // , 2. dynamic particle ... not sure, but would be extremely flexible.

      return sec;
    }
  };

} // namespace corsika::history
