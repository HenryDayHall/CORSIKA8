/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>

namespace corsika {

  Universe::Universe(CoordinateSystemPtr const& pCS)
      : corsika::Sphere(Point{pCS, 0 * meter, 0 * meter, 0 * meter},
                        meter * std::numeric_limits<double>::infinity()) {}

  bool Universe::contains(corsika::Point const&) const { return true; }

} // namespace corsika
