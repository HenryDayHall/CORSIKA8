/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/DensityFunction.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/ShowerAxis.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

using namespace corsika::geometry;
using namespace corsika::environment;
using namespace corsika::particles;
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika;

const auto density = 1_kg / (1_m * 1_m * 1_m);

auto setupEnvironment(particles::Code vTargetCode) {
  // setup environment, geometry
  auto env = std::make_unique<environment::Environment<environment::IMediumModel>>();
  auto& universe = *(env->GetUniverse());
  const geometry::CoordinateSystem& cs = env->GetCoordinateSystem();

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      density, environment::NuclearComposition(std::vector<particles::Code>{vTargetCode},
                                               std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  return std::make_tuple(std::move(env), &cs, nodePtr);
}

TEST_CASE("Homogeneous Density") {
  auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Nitrogen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  auto const observationHeight = 0_km;
  auto const injectionHeight = 10_km;
  auto const t = -observationHeight + injectionHeight;
  Point const showerCore{cs, 0_m, 0_m, observationHeight};
  Point const injectionPos = showerCore + Vector<dimensionless_d>{cs, {0, 0, 1}} * t;

  environment::ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos),
                                           *env, 20};

  CHECK(showerAxis.steplength() == 500_m);

  CHECK(showerAxis.maximumX() / (10_km * density) == Approx(1).epsilon(1e-8));

  CHECK(showerAxis.minimumX() == 0_g / square(1_cm));

  const Point p{cs, 10_km, 20_km, 8.3_km};
  CHECK(showerAxis.projectedX(p) / (1.7_km * density) == Approx(1).epsilon(1e-8));

  const units::si::LengthType d = 6.789_km;
  CHECK(showerAxis.X(d) / (d * density) == Approx(1).epsilon(1e-8));

  const Vector<dimensionless_d> dir{cs, {0, 0, -1}};
  CHECK(showerAxis.GetDirection().GetComponents(cs) == dir.GetComponents(cs));
  CHECK(showerAxis.GetStart().GetCoordinates() == injectionPos.GetCoordinates());
}
