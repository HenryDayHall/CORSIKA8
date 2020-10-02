/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <vector>

namespace corsika::history {

  /**
   * This class stores the non-common properties of secondaries in an event. All
   * other (common) properties are available via the event itself or its projectile.
   */
  struct SecondaryParticle {
    units::si::HEPEnergyType const energy_;
    geometry::Vector<units::si::hepmomentum_d> const momentum_;
    particles::Code const pid_;

  public:
    SecondaryParticle(units::si::HEPEnergyType energy,
                      geometry::Vector<units::si::hepmomentum_d> momentum,
                      particles::Code pid)
        : energy_{energy}
        , momentum_{momentum}
        , pid_{pid} {}
  };

} // namespace corsika::history
