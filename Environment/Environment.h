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
  struct Universe : public corsika::geometry::Volume {
      bool Contains(corsika::geometry::Point const&) const override {return true;}
  };

  class Environment {
  public:
    Environment() : fUniverse(std::make_unique<VolumeTreeNode<IEnvironmentModel>>(
          std::make_unique<Universe>())) {}
  
    using IEnvironmentModel = corsika::setup::IEnvironmentModel;

    auto& GetUniverse() { return fUniverse; }
    auto const& GetUniverse() const { return fUniverse; }

    auto const& GetCoordinateSystem() const {
      return corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCS();
    }

    // factory method for creation of VolumeTreeNodes
    template <class VolumeType, typename... Args>
    static auto CreateNode(Args&&... args) {
      static_assert(std::is_base_of_v<corsika::geometry::Volume, VolumeType>,
                    "unusable type provided, needs to be derived from "
                    "\"corsika::geometry::Volume\"");

      return std::make_unique<VolumeTreeNode<IEnvironmentModel>>(
          std::make_unique<VolumeType>(std::forward<Args>(args)...));
    }

  private:
    VolumeTreeNode<IEnvironmentModel>::VTNUPtr fUniverse;
  };

} // namespace corsika::environment

#endif
