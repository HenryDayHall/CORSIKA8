/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/MediumTypes.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  /**
   * An interface for type of media, needed e.g. to determine energy losses
   *
   * This is the base interface for media types.
   *
   */
  template <typename Model>
  class IMediumTypeModel : public Model {

  public:
    /**
     * Evaluate the medium type at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The media type
     */
    virtual EMediumType medium_type(corsika::geometry::Point const&) const = 0;

    /**
     * A virtual default destructor.
     */
    virtual ~IMediumTypeModel() = default;

  }; // END: class IMediumTypeModel

} // namespace corsika::environment
