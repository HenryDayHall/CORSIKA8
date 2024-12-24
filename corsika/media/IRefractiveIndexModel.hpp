/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  /**
   * An interface for refractive index models.
   *
   * This is the base interface for refractive index mixins.
   *
   */
  template <typename TModel>
  class IRefractiveIndexModel : public TModel {

  public:
    /**
     * Evaluate the refractive index at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this point.
     */
    virtual double getRefractiveIndex(Point const&) const = 0;

    /**
     * A virtual default destructor.
     */
    virtual ~IRefractiveIndexModel() = default;

  }; // END: class IRefractiveIndexModel

} // namespace corsika
