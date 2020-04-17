/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/utility/Singleton.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

/*!
 * This is the only way to get a root-coordinate system, and it is a
 * singleton. All other CoordinateSystems must be relative to the
 * RootCoordinateSystem
 */

namespace corsika {

  class RootCoordinateSystem : public corsika::Singleton<RootCoordinateSystem> {

    friend class corsika::Singleton<RootCoordinateSystem>;

  protected:
    RootCoordinateSystem() {}

  public:
    corsika::CoordinateSystem& GetRootCoordinateSystem()
    {
    	return fRootCS;
    }
    const corsika::CoordinateSystem& GetRootCoordinateSystem() const
    {
      return fRootCS;
    }

  private:
    corsika::CoordinateSystem fRootCS; // THIS IS IT
  };

} // namespace corsika

