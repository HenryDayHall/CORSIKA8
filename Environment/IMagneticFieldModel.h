/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  /**
   * An interface for magnetic field models.
   *
   * This is the base interface for magnetic field model mixins.
   *
   */
  template <typename Model>
  class IMagneticFieldModel : public Model {

    // a type-alias for a magnetic field vector
    using MagneticFieldVector =
        corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>;

  public:
    /**
     * Evaluate the magnetic field at a given location.
     *
     * @param  point    The location to evaluate the field at.
     * @returns    The magnetic field vector at that point.
     */
    virtual auto GetMagneticField(corsika::geometry::Point const&) const
        -> MagneticFieldVector = 0;

    /**
     * A virtual default destructor.
     */
    virtual ~IMagneticFieldModel() = default; // LCOV_EXCL_LINE

  }; // END: class MagneticField

} // namespace corsika::environment
