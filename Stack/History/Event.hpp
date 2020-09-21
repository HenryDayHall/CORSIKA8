/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/history/SecondaryParticle.hpp>

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace corsika::history {

  class Event;
  using EvtPtr = std::shared_ptr<history::Event>;

  class Event {

    size_t projectileIndex_ = 0; //!< reference to projectile (for secondaries_)
    std::vector<SecondaryParticle> secondaries_;
    EvtPtr parent_event_;
    size_t sec_index_ =
        0; //!< index of the projectile of this Event in the parent_event_.secondaries_;

    std::optional<corsika::particles::Code>
        targetCode_; // cannot be const, value set only after construction

  public:
    Event() = default;

    void setParentEvent(EvtPtr& evt) { parent_event_ = evt; }

    void setParentEventAndSecondaryIndex(EvtPtr& evt, size_t sec_index) {
      setParentEvent(evt);
      setParentSecondaryIndex(sec_index);
    }

    void setParentSecondaryIndex(size_t sec_index) { sec_index_ = sec_index; }

    EvtPtr parentEvent() { return parent_event_; }

    void setProjectileIndex(size_t i) { projectileIndex_ = i; }
    size_t projectileIndex() const { return projectileIndex_; }

    template <typename TStackIterator>
    TStackIterator projectile(TStackIterator begin) {
      return begin + projectileIndex_;
    }

    void addSecondary(units::si::HEPEnergyType energy,
                      geometry::Vector<units::si::hepmomentum_d> momentum,
                      particles::Code pid) {
      secondaries_.emplace_back(energy, momentum, pid);
    }
    std::vector<SecondaryParticle>& secondaries() { return secondaries_; }

    void setTargetCode(const particles::Code t) { targetCode_ = t; }
  };

} // namespace corsika::history
