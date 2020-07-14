/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/environment/IRefractiveIndexModel.h>

namespace corsika::environment {

  /**
   * A uniform refractive index.
   *
   * This class returns the same refractive index
   * for all evaluated locations.
   *
   */
  template <typename T>
  class UniformRefractiveIndex : public T {

    double n_; ///< The constant refractive index that we use.

  public:
    /**
     * Construct a UniformRefractiveIndex.
     *
     * This is initialized with a fixed refractive index
     * and returns this refractive index at all locations.
     *
     * @param field    The refractive index to return everywhere.
     */
    template <typename... Args>
    UniformRefractiveIndex(double const n, Args&&... args)
        : T(std::forward<Args>(args)...)
        , n_(n) {}

    /**
     * Evaluate the refractive index at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this point.
     */
    double GetRefractiveIndex(corsika::geometry::Point const&) const final override {
      return n_;
    }

    /**
     * Set the refractive index returned by this instance.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this location.
     */
    void SetRefractiveIndex(double const& n) { n_ = n; }

  }; // END: class RefractiveIndex

} // namespace corsika::environment
