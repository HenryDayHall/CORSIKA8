
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_setup_setuptrajectory_h_
#define _corsika_setup_setuptrajectory_h_

#include <corsika/geometry/Helix.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Trajectory.h>

#include <variant>

namespace corsika::setup {

  using corsika::geometry::Helix;
  using corsika::geometry::Line;

  typedef std::variant<std::monostate,
                       corsika::geometry::Trajectory<Line>,
                       corsika::geometry::Trajectory<Helix>>
      Trajectory;

} // namespace corsika::setup

#endif
