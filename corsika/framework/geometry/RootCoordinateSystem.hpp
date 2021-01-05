/*
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

  /**
   * To refer to CoordinateSystems, only the CoordinateSystemPtr must be used.
   */
  using CoordinateSystemPtr = std::shared_ptr<CoordinateSystem const>;

  /*!
   * Singleton factory function to produce the root CoordinateSystem
   *
   * This is the only way to get a root-coordinate system, and it is a
   * singleton. All other CoordinateSystems must be relative to the
   * RootCoordinateSystem
   */

  static CoordinateSystemPtr& get_root_CoordinateSystem() {
    static CoordinateSystemPtr rootCS(new CoordinateSystem); // THIS IS IT
    return rootCS;
  }

} // namespace corsika
