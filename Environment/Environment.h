/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_environment_Environment_h
#define _include_environment_Environment_h

#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Sphere.h>
#include <limits>

namespace corsika::environment {
  struct Universe : public corsika::geometry::Sphere {
    Universe(corsika::geometry::CoordinateSystem const& pCS)
        : corsika::geometry::Sphere(
              corsika::geometry::Point{pCS, 0 * corsika::units::si::meter,
                                       0 * corsika::units::si::meter,
                                       0 * corsika::units::si::meter},
              corsika::units::si::meter * std::numeric_limits<double>::infinity()) {}

    bool Contains(corsika::geometry::Point const&) const override { return true; }
  };

  template <typename IEnvironmentModel>
  class Environment {
  public:
    using BaseNodeType = VolumeTreeNode<IEnvironmentModel>;

    Environment()
        : fCoordinateSystem{corsika::geometry::RootCoordinateSystem::GetInstance()
                                .GetRootCoordinateSystem()}
        , fUniverse(std::make_unique<BaseNodeType>(
              std::make_unique<Universe>(fCoordinateSystem))) {}

    // using IEnvironmentModel = corsika::setup::IEnvironmentModel;

    auto& GetUniverse() { return fUniverse; }
    auto const& GetUniverse() const { return fUniverse; }

    auto const& GetCoordinateSystem() const { return fCoordinateSystem; }

    // factory method for creation of VolumeTreeNodes
    template <class TVolumeType, typename... TVolumeArgs>
    static auto CreateNode(TVolumeArgs&&... args) {
      static_assert(std::is_base_of_v<corsika::geometry::Volume, TVolumeType>,
                    "unusable type provided, needs to be derived from "
                    "\"corsika::geometry::Volume\"");

      return std::make_unique<BaseNodeType>(
          std::make_unique<TVolumeType>(std::forward<TVolumeArgs>(args)...));
    }

  private:
    corsika::geometry::CoordinateSystem const& fCoordinateSystem;
    typename BaseNodeType::VTNUPtr fUniverse;
  };

  // using SetupBaseNodeType = VolumeTreeNode<corsika::setup::IEnvironmentModel>;
  // using SetupEnvironment = Environment<corsika::setup::IEnvironmentModel>;

} // namespace corsika::environment

#endif
