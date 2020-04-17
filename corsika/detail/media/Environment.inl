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
    auto& Environment<IEnvironmentModel>::GetUniverse() {
 return fUniverse;     }

    template <typename IEnvironmentModel>
    auto const& Environment<IEnvironmentModel>::GetUniverse() const {  return fUniverse; }

    template <typename IEnvironmentModel>
    auto const& Environment<IEnvironmentModel>::GetCoordinateSystem() const { return fCoordinateSystem; }

    // factory method for creation of VolumeTreeNodes
    template <typename IEnvironmentModel>
    template <class TVolumeType, typename... TVolumeArgs>
    auto Environment<IEnvironmentModel>::CreateNode(TVolumeArgs&&... args) {
      static_assert(std::is_base_of_v<corsika::Volume, TVolumeType>,
                    "unusable type provided, needs to be derived from "
                    "\"corsika::Volume\"");

      return std::make_unique<BaseNodeType>(
          std::make_unique<TVolumeType>(std::forward<TVolumeArgs>(args)...));
    }

}