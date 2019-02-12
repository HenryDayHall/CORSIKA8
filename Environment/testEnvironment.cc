
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file

#include <corsika/environment/DensityFunction.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/InhomogeneousMedium.h>
#include <corsika/environment/LinearApproximationIntegrator.h>
#include <corsika/environment/NuclearComposition.h>
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
using namespace corsika::units::si;

TEST_CASE("HomogeneousMedium") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);
}

auto constexpr rho0 = 1_kg / 1_m / 1_m / 1_m;

struct Exponential {
  auto operator()(corsika::geometry::Point const& p) const {
    return exp(p.GetCoordinates()[0] / 1_m) * rho0;
  }

  template <int N>
  auto Derivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return v.GetComponents()[0] * (*this)(p) /
           corsika::units::si::detail::static_pow<N>(1_m);
  }

  auto FirstDerivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return Derivative<1>(p, v);
  }

  auto SecondDerivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return Derivative<2>(p, v);
  }
};

TEST_CASE("InhomogeneousMedium") {
  CoordinateSystem const& cs =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  Point const origin(cs, {0_m, 0_m, 0_m});

  Vector direction(cs, QuantityVector<dimensionless_d>(1, 0, 0));

  Line line(origin, Vector<SpeedType::dimension_type>(
                        cs, {20_m / second, 0_m / second, 0_m / second}));

  auto const tEnd = 5_s;
  Trajectory<Line> const trajectory(line, tEnd);

  Exponential const e;
  DensityFunction<decltype(e), LinearApproximationIntegrator> const rho(e);

  SECTION("DensityFunction") {
    REQUIRE(e.Derivative<1>(origin, direction) / (1_kg / 1_m / 1_m / 1_m / 1_m) ==
            Approx(1));
    REQUIRE(rho.EvaluateAt(origin) == e(origin));
  }

  auto const exactGrammage = [](auto l) { return 1_m * rho0 * (exp(l / 1_m) - 1); };
  auto const exactLength = [](auto X) { return 1_m * log(1 + X / (rho0 * 1_m)); };

  auto constexpr l = 15_cm;

  NuclearComposition const composition{{Code::Proton}, {1.f}};
  InhomogeneousMedium<IMediumModel, decltype(rho)> const inhMedium(composition, rho);

  SECTION("Integration") {
    REQUIRE(rho.IntegrateGrammage(trajectory, l) / exactGrammage(l) ==
            Approx(1).epsilon(1e-2));
    REQUIRE(rho.ArclengthFromGrammage(trajectory, exactGrammage(l)) /
                exactLength(exactGrammage(l)) ==
            Approx(1).epsilon(1e-2));
    REQUIRE(rho.MaximumLength(trajectory, 1e-2) >
            l); // todo: write reasonable test when implementation is working

    REQUIRE(rho.IntegrateGrammage(trajectory, l) ==
            inhMedium.IntegratedGrammage(trajectory, l));
    REQUIRE(rho.ArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)) ==
            inhMedium.ArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)));
  }
}
