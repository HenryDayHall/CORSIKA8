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
#include <optional>
#include <vector>
#include <memory>

namespace corsika::history {

  struct Event {
    size_t const projectileIndex_; //!< reference to projectile
    std::vector<SecondaryParticle> secondaries_;
    //std::shared_ptr<Event> parent_event_;
    
    // meta information, could also be in a separate class
    std::optional<corsika::particles::Code>
        targetCode_; // cannot be const, value set only after construction

  public:
    Event(const size_t projectileIndex)
        : projectileIndex_{projectileIndex} {
      std::cout << "Event created (index = " << projectileIndex_ << ")" << std::endl;
    }
    
    void addSecondary(units::si::HEPEnergyType energy,
                      geometry::Vector<units::si::hepmomentum_d> momentum,
                      particles::Code pid) {
      secondaries_.emplace_back(energy, momentum, pid);
    }
    
    void setTargetCode(const particles::Code t) { targetCode_=t; }
    
  };

  using EvtPtr = std::shared_ptr<history::Event>;
  
} // namespace corsika::history
