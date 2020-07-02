/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  /**
   * An interface for refractive index models.
   *
   * This is the base interface for refractive index mixins.
   *
   */
  template <typename Model>
  class IRefractiveIndexModel : public Model {

  public:
    /**
     * Evaluate the refractive index at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The refractive index at this point.
     */
    virtual double GetRefractiveIndex(corsika::geometry::Point const&) const = 0;

    /**
     * A virtual default destructor.
     */
    virtual ~IRefractiveIndexModel() = default;

  }; // END: class IRefractiveIndexModel

} // namespace corsika::environment
