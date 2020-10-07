/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/SecondaryView.h>
#include <corsika/stack/history/Event.hpp>

#include <corsika/logging/Logging.h>

#include <boost/type_index.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace corsika::history {

  //! mix-in class for SecondaryView that fills secondaries into an \class Event
  template <class T1, template <class> class T2>
  class HistorySecondaryProducer {
  public:
    EventPtr event_;
    static bool constexpr has_event{true};

  public:
    template <typename Particle>
    HistorySecondaryProducer(Particle const& p)
        : event_{std::make_shared<Event>()} {
      C8LOG_TRACE("HistorySecondaryProducer::HistorySecondaryProducer");
      event_->setProjectileIndex(p.GetIndex());
      event_->setParentEvent(p.GetEvent());
    }

    /**
     * Method is called after a new Secondary has been created on the
     * SecondaryView. Extra logic can be introduced here.
     *
     * The input Particle is the new secondary that was produced and
     * is of course a reference into the SecondaryView itself.
     */
    template <typename Particle>
    auto new_secondary(Particle& sec) {
      C8LOG_TRACE("HistorySecondaryProducer::new_secondary(sec)");

      // store particles at production time in Event here
      auto const sec_index =
          event_->addSecondary(sec.GetEnergy(), sec.GetMomentum(), sec.GetPID());
      sec.SetParentEventIndex(sec_index);
      sec.SetEvent(event_);
    }
  };

} // namespace corsika::history
