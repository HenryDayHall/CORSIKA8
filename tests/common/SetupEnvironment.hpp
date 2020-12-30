/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/InhomogeneousMedium.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>

#include <tuple>
#include <memory>

/**
 * \function setup_environment
 *
 * standard environment for unit testing.
 *
 */
namespace corsika::setup::testing {

  inline std::tuple<std::unique_ptr<setup::Environment>, CoordinateSystem const*,
                    setup::Environment::BaseNodeType*>
  setupEnvironment(Code const vTargetCode,
                   MagneticFluxType const& BfieldZ = MagneticFluxType::zero()) {

    auto env = std::make_unique<setup::Environment>();
    auto& universe = *(env->getUniverse());
    CoordinateSystemPtr const& cs = env->getCoordinateSystem();

    /**
     * our world is a sphere at 0,0,0 with R=infty
     */
    auto world = setup::Environment::createNode<Sphere>(Point{cs, 0_m, 0_m, 0_m}, 100_km);

    /**
     * construct suited environment medium model:
     */
    using MyHomogeneousModel = MediumPropertyModel<
        UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

    world->setModelProperties<MyHomogeneousModel>(
        Medium::AirDry1Atm, Vector(cs, 0_T, 0_T, BfieldZ), 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition(std::vector<Code>{vTargetCode}, std::vector<float>{1.}));

    setup::Environment::BaseNodeType* nodePtr = world.get();
    universe.addChild(std::move(world));

    return std::make_tuple(std::move(env), &cs, nodePtr);
  }
} // namespace corsika::setup::testing
