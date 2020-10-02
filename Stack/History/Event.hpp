/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/logging/Logging.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/history/EventType.hpp>
#include <corsika/stack/history/SecondaryParticle.hpp>

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace corsika::history {

  class Event;
  using EventPtr = std::shared_ptr<history::Event>;

  class Event {

    size_t projectile_index_ = 0; //!< index of projectile on stack
    std::vector<SecondaryParticle> secondaries_;
    EventPtr parent_event_;

    EventType type_ = EventType::Uninitialized;

    std::optional<corsika::particles::Code> targetCode_;

  public:
    Event() = default;

    void setParentEvent(EventPtr const& evt) { parent_event_ = evt; }

    bool hasParentEvent() const { return bool(parent_event_); }
    EventPtr& parentEvent() { return parent_event_; }
    EventPtr const& parentEvent() const { return parent_event_; }

    void setProjectileIndex(size_t i) { projectile_index_ = i; }
    size_t projectileIndex() const { return projectile_index_; }

    template <typename TStackIterator>
    TStackIterator projectile(TStackIterator begin) {
      // todo: change this
      // MR: This is dangerous. You can pass any iterator though it must
      // be stack.begin() to yield the correct projectile

      return begin + projectile_index_;
    }

    size_t addSecondary(units::si::HEPEnergyType energy,
                        geometry::Vector<units::si::hepmomentum_d> const& momentum,
                        particles::Code pid) {
      secondaries_.emplace_back(energy, momentum, pid);
      return secondaries_.size() - 1;
    }

    std::vector<SecondaryParticle> const& secondaries() const { return secondaries_; }

    void setTargetCode(const particles::Code t) { targetCode_ = t; }

    std::string as_string() const {
      return fmt::format("hasParent={}, projIndex={}, Nsec={}", hasParentEvent(),
                         projectile_index_, secondaries_.size());
    }

    EventType eventType() const { return type_; }

    void setEventType(EventType t) { type_ = t; }
  };

} // namespace corsika::history
