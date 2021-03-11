/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <bits/stdc++.h>
#include <corsika/media/IRefractiveIndexModel.hpp>

namespace corsika {

  /**
   * An exponential refractive index.
   *
   * This class returns the value of an exponential refractive index
   * for all evaluated locations.
   *
   */
  template <typename T>
  class ExponentialRefractiveIndex : public T {

    double n_0;                ///< n0 constant.
    InverseLengthType lambda_; ///< lambda parameter.

  public:
    /**
     * Construct an ExponentialRefractiveIndex.
     *
     * This is initialized with two parameters n_0 and lambda
     * and returns the value of the exponential refractive index
     * at a given point location.
     *
     * @param field    The refractive index to return to a given point.
     */
    template <typename... Args>
    ExponentialRefractiveIndex(double const n0, InverseLengthType const lambda,
                               Args&&... args);

    /**
     * Evaluate the refractive index at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this point.
     */
    double getRefractiveIndex(Point const& point) const final override;

  }; // END: class ExponentialRefractiveIndex

} // namespace corsika

#include <corsika/detail/media/ExponentialRefractiveIndex.inl>
