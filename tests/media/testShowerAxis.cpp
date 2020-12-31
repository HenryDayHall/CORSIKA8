/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/DensityFunction.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

// using namespace
using namespace corsika;

const auto density = 1_kg / (1_m * 1_m * 1_m);

auto setupEnvironment(Code vTargetCode) {
  // setup environment, geometry
  auto env = std::make_unique<Environment<IMediumModel>>();
  auto& universe = *(env->getUniverse());
  const CoordinateSystemPtr& cs = env->getCoordinateSystem();

  auto theMedium = Environment<IMediumModel>::createNode<Sphere>(
      Point{cs, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = HomogeneousMedium<IMediumModel>;
  theMedium->setModelProperties<MyHomogeneousModel>(
      density,
      NuclearComposition(std::vector<Code>{vTargetCode}, std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.addChild(std::move(theMedium));

  return std::make_tuple(std::move(env), &cs, nodePtr);
}

TEST_CASE("Homogeneous Density") {
  auto [env, csPtr, nodePtr] = setupEnvironment(Code::Nitrogen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  auto const observationHeight = 0_km;
  auto const injectionHeight = 10_km;
  auto const t = -observationHeight + injectionHeight;
  Point const showerCore{cs, 0_m, 0_m, observationHeight};
  Point const injectionPos = showerCore + Vector<dimensionless_d>{cs, {0, 0, 1}} * t;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos), *env,
                              false, // -> do not throw exceptions
                              20};   // -> number of bins

  CHECK(showerAxis.getSteplength() == 500_m);

  CHECK(showerAxis.getMaximumX() / (10_km * density) == Approx(1).epsilon(1e-8));

  CHECK(showerAxis.getMinimumX() == 0_g / square(1_cm));

  const Point p{cs, 10_km, 20_km, 8.3_km};
  CHECK(showerAxis.getProjectedX(p) / (1.7_km * density) == Approx(1).epsilon(1e-8));

  const LengthType d = 6.789_km;
  CHECK(showerAxis.getX(d) / (d * density) == Approx(1).epsilon(1e-8));

  const Vector<dimensionless_d> dir{cs, {0, 0, -1}};
  CHECK(showerAxis.getDirection().getComponents(cs) == dir.getComponents(cs));
  CHECK(showerAxis.getStart().getCoordinates() == injectionPos.getCoordinates());
}
