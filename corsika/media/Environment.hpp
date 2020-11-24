/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Sphere.hpp>

#include <corsika/media/Universe.hpp>

#include <limits>

namespace corsika {

  template <typename IEnvironmentModel>
  class Environment {
  public:
    using BaseNodeType = VolumeTreeNode<IEnvironmentModel>;

    Environment();

    typename BaseNodeType::VTNUPtr& getUniverse();
    typename BaseNodeType::VTNUPtr const& getUniverse() const;

    CoordinateSystem const& getCoordinateSystem() const;

    // factory method for creation of VolumeTreeNodes
    template <class TVolumeType, typename... TVolumeArgs>
    static std::unique_ptr<BaseNodeType> CreateNode(TVolumeArgs&&... args);

  private:
    CoordinateSystem const& coordinateSystem_;
    typename BaseNodeType::VTNUPtr universe_;
  };

} // namespace corsika

#include <corsika/detail/media/Environment.inl>
