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
#include <set>

namespace corsika {

  // fwd decl
  template <typename IEnvironmentModel>
  class Environment;

  template <typename IEnvironmentModel>
  std::set<Code> const get_all_elements_in_universe(
      Environment<IEnvironmentModel> const& env);

  /**
   * Base Environment class.
   *
   * Describes the Environment in which the shower is propagated.
   */
  template <typename IEnvironmentModel>
  class Environment {
  public:
    using BaseNodeType = VolumeTreeNode<IEnvironmentModel>;

    Environment();

    /**
     * Getters for the universe stored in the Environment.
     *
     * @retval Retuns reference to a Universe object with infinite size.
     */
    ///@{
    //! Get non const universe
    typename BaseNodeType::VTNUPtr& getUniverse();
    //! Get const universe
    typename BaseNodeType::VTNUPtr const& getUniverse() const;
    ///@}

    /**
     * Getter for the CoordinateSystem used in the Environment.
     *
     * @retval Retuns a const reference to the CoordinateSystem used.
     */
    CoordinateSystemPtr const& getCoordinateSystem() const;

    /**
     * Factory method for creation of VolumeTreeNodes.
     *
     * @tparam TVolumeType Type of volume to be created
     * @tparam TVolumeArgs Types to forward to the constructor
     * @param args Parameter forwarded to the constructor of TVolumeType
     * @retval Returns unique pointer to a VolumeTreeNode with the same EnvitonmentModel
     * as this class.
     */
    template <class TVolumeType, typename... TVolumeArgs>
    static std::unique_ptr<BaseNodeType> createNode(TVolumeArgs&&... args);

  private:
    CoordinateSystemPtr const coordinateSystem_;
    typename BaseNodeType::VTNUPtr universe_;
  };

} // namespace corsika

#include <corsika/detail/media/Environment.inl>
