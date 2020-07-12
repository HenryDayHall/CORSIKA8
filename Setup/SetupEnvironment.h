/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMagneticFieldModel.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/IMediumPropertyModel.h>
#include <corsika/environment/IRefractiveIndexModel.h>
#include <corsika/environment/IMagneticFieldModel.h>

namespace corsika::setup {

  /**
     Definition of the default environemnt model interface. Each model
     interface provides properties of the environment in a position
     bdependent way.
   */

  using EnvironmentInterface = environment::IMediumPropertyModel<
      environment::IMagneticFieldModel<environment::IMediumModel>>;
  using Environment = environment::Environment<EnvironmentInterface>;

} // end namespace corsika::setup

#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/InhomogeneousMedium.h>
#include <corsika/environment/MediumPropertyModel.h>
#include <corsika/environment/UniformMagneticField.h>

/**
 * standard environment for unit testing. This can be moved to
 * "test" directory, when available.
 */
namespace corsika::setup::testing {

  inline auto setupEnvironment(particles::Code vTargetCode) {

    using namespace corsika::units::si;
    using namespace corsika;

    auto env = std::make_unique<setup::Environment>();
    auto& universe = *(env->GetUniverse());
    const geometry::CoordinateSystem& cs = env->GetCoordinateSystem();

    /**
     * our world is a sphere at 0,0,0 with R=infty
     */
    auto world = setup::Environment::CreateNode<geometry::Sphere>(
        geometry::Point{cs, 0_m, 0_m, 0_m},
        1_km * std::numeric_limits<double>::infinity());

    /**
     * construct suited environment medium model:
     */
    using MyHomogeneousModel =
        environment::MediumPropertyModel<environment::UniformMagneticField<
            environment::HomogeneousMedium<setup::EnvironmentInterface>>>;

    world->SetModelProperties<MyHomogeneousModel>(
        environment::Medium::AirDry1Atm, geometry::Vector(cs, 0_T, 0_T, 1_T),
        1_kg / (1_m * 1_m * 1_m),
        environment::NuclearComposition(std::vector<particles::Code>{vTargetCode},
                                        std::vector<float>{1.}));

    auto const* nodePtr = world.get();
    universe.AddChild(std::move(world));

    return std::make_tuple(std::move(env), &cs, nodePtr);
  }

} // namespace corsika::setup::testing
