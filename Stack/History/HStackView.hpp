/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/history/Event.hpp>

#include <memory>

namespace corsika::history {

  template <typename TStackView>
  class HStackView : public TStackView {
    std::shared_ptr<Event<typename TStackView::StackIteratorValue>> event_;

  public:
    template <typename... Args>
    EventBuilder(Args&&... args)
        : TStackView{std::forward<Args>(args)}
        , event_{std::make_shared<Event>()} {}

    template <typename... Args>
    void AddSecondary(Args&&... args) {
      auto const s = TStackView::AddSecondary(
          std::forward<Args>(args), event_); // what if event is not last argument?
      event_->secondaries.emplace_back(s.GetEnergy(), s.GetMomentum(), s.GetParticleID());
    }

    void SetTarget(corsika::particles::ParticleCode targetCode) {
      event_->targetCode = targetCode;
    }
  };

} // namespace corsika::history
