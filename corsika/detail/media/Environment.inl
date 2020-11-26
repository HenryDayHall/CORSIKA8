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

  template <typename IEnvironmentModel>
  Environment<IEnvironmentModel>::Environment()
      : coordinateSystem_{RootCoordinateSystem::getInstance().GetRootCoordinateSystem()}
      , universe_(std::make_unique<BaseNodeType>(
            std::make_unique<Universe>(coordinateSystem_))) {}

  template <typename IEnvironmentModel>
  typename Environment<IEnvironmentModel>::BaseNodeType::VTNUPtr& Environment<IEnvironmentModel>::getUniverse() {
    return universe_;
  }

  template <typename IEnvironmentModel>
  typename Environment<IEnvironmentModel>::BaseNodeType::VTNUPtr const& Environment<IEnvironmentModel>::getUniverse() const {
    return universe_;
  }

  template <typename IEnvironmentModel>
  CoordinateSystem const& Environment<IEnvironmentModel>::getCoordinateSystem() const {
    return coordinateSystem_;
  }

  // factory method for creation of VolumeTreeNodes
  template <typename IEnvironmentModel>
  template <class TVolumeType, typename... TVolumeArgs>
  std::unique_ptr< VolumeTreeNode<IEnvironmentModel> > Environment<IEnvironmentModel>::createNode(TVolumeArgs&&... args) {
    static_assert(std::is_base_of_v<corsika::Volume, TVolumeType>,
                  "unusable type provided, needs to be derived from "
                  "\"corsika::Volume\"");

    return std::make_unique<BaseNodeType>(
        std::make_unique<TVolumeType>(std::forward<TVolumeArgs>(args)...));
  }

} // namespace corsika