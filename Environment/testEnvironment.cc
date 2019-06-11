/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/DensityFunction.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/InhomogeneousMedium.h>
#include <corsika/environment/LinearApproximationIntegrator.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/SlidingPlanarExponential.h>
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
using namespace corsika;

CoordinateSystem const& gCS =
    RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

Point const gOrigin(gCS, {0_m, 0_m, 0_m});

TEST_CASE("HomogeneousMedium") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);
}

TEST_CASE("FlatExponential") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / units::si::detail::static_pow<3>(1_cm);
  FlatExponential<IMediumModel> const medium(gOrigin, axis, rho0, lambda,
                                             protonComposition);
  auto const tEnd = 5_s;

  SECTION("horizontal") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {20_cm / second, 0_m / second, 0_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);

    REQUIRE((medium.IntegratedGrammage(trajectory, 2_m) / (rho0 * 2_m)) == Approx(1));
    REQUIRE((medium.ArclengthFromGrammage(trajectory, rho0 * 5_m) / 5_m) == Approx(1));
  }

  SECTION("vertical") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, 5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);
    LengthType const length = 2 * lambda;
    GrammageType const exact = rho0 * lambda * (exp(length / lambda) - 1);

    REQUIRE((medium.IntegratedGrammage(trajectory, length) / exact) == Approx(1));
    REQUIRE((medium.ArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }

  SECTION("escape grammage") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, -5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);

    GrammageType const escapeGrammage = rho0 * lambda;

    REQUIRE(trajectory.NormalizedDirection().dot(axis).magnitude() < 0);
    REQUIRE(medium.ArclengthFromGrammage(trajectory, 1.2 * escapeGrammage) ==
            std::numeric_limits<typename GrammageType::value_type>::infinity() * 1_m);
  }

  SECTION("inclined") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 5_m / second, 5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);
    double const cosTheta = M_SQRT1_2;
    LengthType const length = 2 * lambda;
    GrammageType const exact =
        rho0 * lambda * (exp(cosTheta * length / lambda) - 1) / cosTheta;
    REQUIRE((medium.IntegratedGrammage(trajectory, length) / exact) == Approx(1));
    REQUIRE((medium.ArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }
}

TEST_CASE("SlidingPlanarExponential") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / units::si::detail::static_pow<3>(1_cm);
  auto const tEnd = 5_s;

  SlidingPlanarExponential<IMediumModel> const medium(gOrigin, rho0, lambda,
                                                      protonComposition);

  SECTION("density") {
    CHECK(medium.GetMassDensity({gCS, {0_m, 0_m, 3_m}}) /
              medium.GetMassDensity({gCS, {0_m, 3_m, 0_m}}) ==
          Approx(1));
  }

  SECTION("vertical") {
    Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
    FlatExponential<IMediumModel> const flat(gOrigin, axis, rho0, lambda,
                                             protonComposition);
    Line const line({gCS, {0_m, 0_m, 1_m}},
                    Vector<SpeedType::dimension_type>(
                        gCS, {0_m / second, 0_m / second, 5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);

    CHECK(medium.GetMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude() ==
          flat.GetMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude());
    CHECK(medium.IntegratedGrammage(trajectory, 2_m).magnitude() ==
          flat.IntegratedGrammage(trajectory, 2_m).magnitude());
    CHECK(medium.ArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude() ==
          flat.ArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude());
  }
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
  Vector direction(gCS, QuantityVector<dimensionless_d>(1, 0, 0));

  Line line(gOrigin, Vector<SpeedType::dimension_type>(
                         gCS, {20_m / second, 0_m / second, 0_m / second}));

  auto const tEnd = 5_s;
  Trajectory<Line> const trajectory(line, tEnd);

  Exponential const e;
  DensityFunction<decltype(e), LinearApproximationIntegrator> const rho(e);

  SECTION("DensityFunction") {
    REQUIRE(e.Derivative<1>(gOrigin, direction) / (1_kg / 1_m / 1_m / 1_m / 1_m) ==
            Approx(1));
    REQUIRE(rho.EvaluateAt(gOrigin) == e(gOrigin));
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
