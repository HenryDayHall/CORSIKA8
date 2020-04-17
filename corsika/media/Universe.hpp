/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Sphere.hpp>
#include <limits>

namespace corsika {

  struct Universe : public corsika::Sphere
  {
    Universe(corsika::CoordinateSystem const& pCS);
    inline bool Contains(corsika::Point const&) const override;
  };

}

#include <corsika/detail/media/Universe.inl>
