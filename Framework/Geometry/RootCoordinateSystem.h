/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_geometry_rootcoordinatesystem_h_
#define _include_corsika_geometry_rootcoordinatesystem_h_

#include <corsika/utl/Singleton.h>

#include <corsika/geometry/CoordinateSystem.h>

/*!
 * This is the only way to get a root-coordinate system, and it is a
 * singleton. All other CoordinateSystems must be relative to the
 * RootCoordinateSystem
 */

namespace corsika::geometry {

  class RootCoordinateSystem : public corsika::utl::Singleton<RootCoordinateSystem> {

    friend class corsika::utl::Singleton<RootCoordinateSystem>;

  protected:
    RootCoordinateSystem() {}

  public:
    corsika::geometry::CoordinateSystem& GetRootCoordinateSystem() { return fRootCS; }
    const corsika::geometry::CoordinateSystem& GetRootCoordinateSystem() const {
      return fRootCS;
    }

  private:
    corsika::geometry::CoordinateSystem fRootCS; // THIS IS IT
  };

} // namespace corsika::geometry

#endif
