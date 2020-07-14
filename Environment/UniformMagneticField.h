/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/environment/IMagneticFieldModel.h>

namespace corsika::environment {

  /**
   * A uniform (constant) magnetic field.
   *
   * This class returns the same magnetic field vector
   * for all evaluated locations.
   *
   */
  template <typename T>
  class UniformMagneticField : public T {

    // a type-alias for a magnetic field vector
    using MagneticFieldVector =
        corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>;

    MagneticFieldVector B_; ///< The constant magnetic field we use.

  public:
    /**
     * Construct a UniformMagneticField.
     *
     * This is initialized with a fixed magnetic field
     * and returns this magnetic field at all locations.
     *
     * @param field    The fixed magnetic field to return.
     */
    template <typename... Args>
    UniformMagneticField(MagneticFieldVector const& B, Args&&... args)
        : T(std::forward<Args>(args)...)
        , B_(B) {}

    /**
     * Evaluate the magnetic field at a given location.
     *
     * @param  point    The location to evaluate the field at.
     * @returns    The magnetic field vector.
     */
    MagneticFieldVector GetMagneticField(
        corsika::geometry::Point const&) const final override {
      return B_;
    }

    /**
     * Set the magnetic field returned by this instance.
     *
     * @param  point    The location to evaluate the field at.
     * @returns    The magnetic field vector.
     */
    auto SetMagneticField(MagneticFieldVector const& Bfield) -> void { B_ = Bfield; }

  }; // END: class MagneticField

} // namespace corsika::environment
