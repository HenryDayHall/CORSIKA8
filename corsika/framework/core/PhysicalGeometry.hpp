/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>

/**
 * \file PhysicalUnits.hpp
 *
 * Import and extend the phys::units package. The SI units are also imported into the
 * `\namespace corsika`, since they are used everywhere as integral part of the framework.
 */

namespace corsika {

  /**
   * A 3D vector defined in a specific coordinate system with units HEPMomentumType
   **/
  typedef Vector<hepmomentum_d> MomentumVector;

  /**
   * A 3D vector defined in a specific coordinate system with no units. But, note, this is
   * not automatically normaliyed! It is not a "NormalVector".
   **/
  typedef Vector<dimensionless_d> DirectionVector;

  /**
   * A 3D vector defined in a specific coordinate system with units "velocity_t".
   *
   **/
  typedef Vector<SpeedType::dimension_type> VelocityVector;

} // namespace corsika
