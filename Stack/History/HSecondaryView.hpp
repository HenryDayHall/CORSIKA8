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

  namespace detail {
    template <typename TParticleIterator>
    using SecondaryViewTypeFromIterator =
        decltype(stack::SecondaryView{std::declval<TParticleIterator&>()});
  }

  template <typename TParticleIterator>
  class HSecondaryView : public detail::SecondaryViewTypeFromIterator<TParticleIterator> {
    std::shared_ptr<Event> event_;

    using BaseSecondaryViewType =
        detail::SecondaryViewTypeFromIterator<TParticleIterator>;

  public:
    HSecondaryView(TParticleIterator& p)
        : BaseSecondaryViewType{p}
        , event_{std::make_shared<Event>(p.GetIndex())} {}

    template <typename... Args>
    void AddSecondary(Args&&... args) {
      auto const s = BaseSecondaryViewType::AddSecondary(
          std::forward<Args...>(args...), event_); // what if event is not last argument?
      event_->secondaries.emplace_back(s.GetEnergy(), s.GetMomentum(), s.GetParticleID());
    }

    void SetTarget(particles::Code targetCode) { event_->targetCode = targetCode; }
  };

} // namespace corsika::history
