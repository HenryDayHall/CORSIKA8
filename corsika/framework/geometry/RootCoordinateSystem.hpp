n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <memory>

namespace corsika {

  /*!
   * Singleton factory function to produce the root CoordinateSystem
   *
   * This is the only way to get a root-coordinate system, and it is a
   * singleton. All other CoordinateSystems must be relative to the
   * RootCoordinateSystem
   */

  static std::shared_ptr<CoordinateSystem const> get_root_CoordinateSystem() {
    static std::shared_ptr<CoordinateSystem const> rootCS(
        new CoordinateSystem); // THIS IS IT
    return rootCS;
  }

} // namespace corsika
