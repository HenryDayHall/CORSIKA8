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

#include <corsika/media/Environment.hpp>

namespace corsika {

  Universe::Universe(corsika::CoordinateSystem const& pCS)
      : corsika::Sphere(corsika::Point{pCS, 0 * meter, 0 * meter, 0 * meter},
                        meter * std::numeric_limits<double>::infinity()) {}

  bool Universe::Contains(corsika::Point const&) const { return true; }

} // namespace corsika