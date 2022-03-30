/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IMediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/HomogeneousMedium.hpp>

#include <limits>

namespace corsika {

  using DummyEnvironmentInterface =
      IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
  using DummyEnvironment = Environment<DummyEnvironmentInterface>;

  namespace setup::testing {

    /**
     * \function setup_environment
     *
     * standard environment for unit testing.
     *
     */

    inline std::tuple<std::unique_ptr<DummyEnvironment>, CoordinateSystemPtr const*,
                      DummyEnvironment::BaseNodeType*>
    setup_environment(Code const vTargetCode,
                      MagneticFluxType const& BfieldZ = MagneticFluxType::zero()) {

      auto env = std::make_unique<DummyEnvironment>();
      auto& universe = *(env->getUniverse());
      CoordinateSystemPtr const& cs = env->getCoordinateSystem();

      /**
       * our world is a sphere at 0,0,0 with R=infty
       */
      auto world = DummyEnvironment::createNode<Sphere>(Point{cs, 0_m, 0_m, 0_m}, 100_km);

      /**
       * construct suited environment medium model:
       */
      using MyHomogeneousModel = MediumPropertyModel<
          UniformMagneticField<HomogeneousMedium<DummyEnvironmentInterface>>>;

      world->setModelProperties<MyHomogeneousModel>(
          Medium::AirDry1Atm, Vector(cs, 0_T, 0_T, BfieldZ), 1_kg / (1_m * 1_m * 1_m),
          NuclearComposition(std::vector<Code>{vTargetCode}, std::vector<double>{1.}));

      DummyEnvironment::BaseNodeType* nodePtr = world.get();
      universe.addChild(std::move(world));

      return std::make_tuple(std::move(env), &cs, nodePtr);
    }

  } // namespace setup::testing

} // namespace corsika
