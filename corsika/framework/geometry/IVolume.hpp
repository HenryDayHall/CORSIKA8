/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  class IVolume {

  public:
    //! returns true if the Point p is within the volume
    virtual bool isInside(Point const& p) const = 0;

    virtual ~IVolume() = default;
  };

} // namespace corsika
