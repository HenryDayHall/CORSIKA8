#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <limits>

namespace corsika::setup::testing {

  inline std::tuple<std::unique_ptr<setup::Environment>, CoordinateSystem const*,
                    setup::Environment::BaseNodeType const*>
  setup_environment(Code vTargetCode) {

    auto env = std::make_unique<setup::Environment>();
    auto& universe = *(env->GetUniverse());
    CoordinateSystem const& cs = env->GetCoordinateSystem();

    /**
     * our world is a sphere at 0,0,0 with R=infty
     */
    auto world = setup::Environment::CreateNode<Sphere>(
        Point{cs, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    /**
     * construct suited environment medium model:
     */
    using MyHomogeneousModel = MediumPropertyModel<
        UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

    world->SetModelProperties<MyHomogeneousModel>(
        Medium::AirDry1Atm, Vector(cs, 0_T, 0_T, 1_T), 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition(std::vector<Code>{vTargetCode}, std::vector<float>{1.}));

    setup::Environment::BaseNodeType const* nodePtr = world.get();
    universe.AddChild(std::move(world));

    return std::make_tuple(std::move(env), &cs, nodePtr);
  }

} // namespace corsika::setup::testing
