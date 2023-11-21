/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  inline SeparationPlane::SeparationPlane(Plane const& plane)
      : plane_(plane) {}

  inline bool SeparationPlane::contains(Point const& p) const {
    return !plane_.isAbove(p);
  }

  inline std::string SeparationPlane::asString() const { return plane_.asString(); }

} // namespace corsika