/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_h
#define _include_Environment_h

#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/setup/SetupEnvironment.h>

namespace corsika::environment {

  class Environment {
  public:
    auto& GetUniverse() { return universe; }
    auto const& GetCS() const { return corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCS();}

  private:
    VolumeTreeNode<corsika::setup::IEnvironmentModel>::VTNUPtr universe;
  };

} // namespace corsika::environment

#endif
