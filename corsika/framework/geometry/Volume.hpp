/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  class Volume {

  public:
    //! returns true if the Point p is within the volume
    virtual bool Contains(Point const& p) const = 0;

    virtual ~Volume() = default;
  };

} // namespace corsika
