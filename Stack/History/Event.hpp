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

namespace corsika::history {

  struct Event {
    size_t const projectileIndex_; //!< reference to projectile
    std::vector<SecondaryParticle> secondaries;

    // meta information, could also be in a separate class
    std::optional<corsika::particles::Code>
        targetCode; // cannot be const, value set only after construction

  public:
    Event(size_t projectileIndex)
        : projectileIndex_{projectileIndex} {
      std::cout << "Event created (index = " << projectileIndex_ << ")" << std::endl;
    }
  };

} // namespace corsika::history
