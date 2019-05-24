/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_VOLUME_H_
#define _include_VOLUME_H_

#include <corsika/geometry/Point.h>

namespace corsika::geometry {

  class Volume {

  public:
    //! returns true if the Point p is within the volume
    virtual bool Contains(Point const& p) const = 0;

    virtual ~Volume() = default;
  };

} // namespace corsika::geometry

#endif
