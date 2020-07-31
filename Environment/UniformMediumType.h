/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/IMediumTypeModel.h>
#include <corsika/environment/MediumTypes.h>

namespace corsika::environment {

  /**
   * A uniform refractive index.
   *
   * This class returns the same refractive index
   * for all evaluated locations.
   *
   */
  template <typename T>
  class UniformMediumType : public T {

    EMediumType type_; ///< The constant medium type

  public:
    /**
     * Construct a UniformMediumType.
     *
     * This is initialized with a fixed medium type
     * and returns this at all locations.
     *
     * @param field    The medium type to return everywhere.
     */
    template <typename... Args>
    UniformMediumType(EMediumType const type, Args&&... args)
        : T(std::forward<Args>(args)...)
        , type_(type) {}

    /**
     * Evaluate the refractive index at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this point.
     */
    EMediumType medium_type(corsika::geometry::Point const&) const final override {
      return type_;
    }

    /**
     * Set the refractive index returned by this instance.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this location.
     */
    void set_medium_type(EMediumType const& type) { type_ = type; }

  }; // END: class MediumType

} // namespace corsika::environment
