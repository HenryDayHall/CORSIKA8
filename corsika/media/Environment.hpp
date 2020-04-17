/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Universe.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <limits>

namespace corsika {

  template <typename IEnvironmentModel>
  class Environment
  {
  public:
	  using BaseNodeType = VolumeTreeNode<IEnvironmentModel>;

	  Environment():
		fCoordinateSystem(
		    corsika::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem()),
        fUniverse(
        	std::make_unique<BaseNodeType>(std::make_unique<Universe>(fCoordinateSystem)))
        {}

    // using IEnvironmentModel = corsika::IEnvironmentModel;

    inline auto& GetUniverse();

    inline auto const& GetUniverse() const ;

    inline auto const& GetCoordinateSystem() const ;

    // factory method for creation of VolumeTreeNodes
    template <class TVolumeType, typename... TVolumeArgs>
    static auto CreateNode(TVolumeArgs&&... args);


  private:

    corsika::CoordinateSystem const& fCoordinateSystem;
    typename BaseNodeType::VTNUPtr fUniverse;

  };

  // using SetupBaseNodeType = VolumeTreeNode<corsika::IEnvironmentModel>;
  // using SetupEnvironment = Environment<corsika::IEnvironmentModel>;

} // namespace corsika::environment


#include <corsika/detail/media/Environment.inl>
