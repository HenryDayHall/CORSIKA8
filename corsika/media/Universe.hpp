/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Sphere.hpp>
#include <limits>

namespace corsika {

  struct Universe : public corsika::Sphere {
    inline Universe(corsika::CoordinateSystemPtr const& pCS);
    inline bool isInside(corsika::Point const&) const override;
  };

} // namespace corsika

#include <corsika/detail/media/Universe.inl>
