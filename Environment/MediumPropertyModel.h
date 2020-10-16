/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/IMediumPropertyModel.h>

namespace corsika::environment {

  /**
   * A uniform refractive index.
   *
   * This class returns the same refractive index
   * for all evaluated locations.
   *
   */
  template <typename T>
  class MediumPropertyModel : public T {

    Medium medium_; ///< The medium code

  public:
    /**
     * Construct a MediumPropertyModel
     *
     * This is initialized with a fixed refractive index
     * and returns this refractive index at all locations.
     *
     * @param field    The refractive index to return everywhere.
     */
    template <typename... Args>
    MediumPropertyModel(const Medium medium, Args&&... args)
        : T(std::forward<Args>(args)...)
        , medium_(medium) {}

    /**
     * Evaluate the medium type at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The medium type as enum environment::Medium
     */
    Medium medium(corsika::geometry::Point const&) const final override {
      return medium_;
    }

    void set_medium(Medium v) { medium_ = v; }

  }; // END: class MediumPropertyModel

} // namespace corsika::environment
