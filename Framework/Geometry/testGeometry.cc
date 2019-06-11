/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/Helix.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

using namespace corsika::geometry;
using namespace corsika::units::si;

double constexpr absMargin = 1.0e-8;

TEST_CASE("transformations between CoordinateSystems") {
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  REQUIRE(CoordinateSystem::GetTransformation(rootCS, rootCS)
              .isApprox(EigenTransform::Identity()));

  QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
  Point p1(rootCS, coordinates);

  QuantityVector<magnetic_flux_density_d> components{1. * tesla, 0. * tesla, 0. * tesla};
  Vector<magnetic_flux_density_d> v1(rootCS, components);

  REQUIRE((p1.GetCoordinates() - coordinates).norm().magnitude() ==
          Approx(0).margin(absMargin));
  REQUIRE((p1.GetCoordinates(rootCS) - coordinates).norm().magnitude() ==
          Approx(0).margin(absMargin));

  /*
  SECTION("unconnected CoordinateSystems") {
    CoordinateSystem rootCS2 = CoordinateSystem::CreateRootCS();
    REQUIRE_THROWS(CoordinateSystem::GetTransformation(rootCS, rootCS2));
    }*/

  SECTION("translations") {
    QuantityVector<length_d> const translationVector{0_m, 4_m, 0_m};

    CoordinateSystem translatedCS = rootCS.translate(translationVector);

    REQUIRE(translatedCS.GetReference() == &rootCS);

    REQUIRE((p1.GetCoordinates(translatedCS) + translationVector).norm().magnitude() ==
            Approx(0).margin(absMargin));

    // Vectors are not subject to translations
    REQUIRE(
        (v1.GetComponents(rootCS) - v1.GetComponents(translatedCS)).norm().magnitude() ==
        Approx(0).margin(absMargin));

    Point p2(translatedCS, {0_m, 0_m, 0_m});
    REQUIRE(((p2 - p1).GetComponents() - translationVector).norm().magnitude() ==
            Approx(0).margin(absMargin));
  }

  SECTION("multiple translations") {
    QuantityVector<length_d> const tv1{0_m, 5_m, 0_m};
    CoordinateSystem cs2 = rootCS.translate(tv1);

    QuantityVector<length_d> const tv2{3_m, 0_m, 0_m};
    CoordinateSystem cs3 = rootCS.translate(tv2);

    QuantityVector<length_d> const tv3{0_m, 0_m, 2_m};
    CoordinateSystem cs4 = cs3.translate(tv3);

    REQUIRE(cs4.GetReference()->GetReference() == &rootCS);

    REQUIRE(CoordinateSystem::GetTransformation(cs3, cs2).isApprox(
        rootCS.translate({3_m, -5_m, 0_m}).GetTransform()));
    REQUIRE(CoordinateSystem::GetTransformation(cs2, cs3).isApprox(
        rootCS.translate({-3_m, +5_m, 0_m}).GetTransform()));
  }

  SECTION("rotations") {
    QuantityVector<length_d> const axis{0_m, 0_m, 1_km};
    double const angle = 90. / 180. * M_PI;

    CoordinateSystem rotatedCS = rootCS.rotate(axis, angle);
    REQUIRE(rotatedCS.GetReference() == &rootCS);

    REQUIRE(v1.GetComponents(rotatedCS)[1].magnitude() ==
            Approx((-1. * tesla).magnitude()));

    // vector norm invariant under rotation
    REQUIRE(v1.GetComponents(rotatedCS).norm().magnitude() ==
            Approx(v1.GetComponents(rootCS).norm().magnitude()));
  }

  SECTION("multiple rotations") {
    QuantityVector<length_d> const zAxis{0_m, 0_m, 1_km};
    QuantityVector<length_d> const yAxis{0_m, 7_nm, 0_m};
    QuantityVector<length_d> const xAxis{2_m, 0_nm, 0_m};

    QuantityVector<magnetic_flux_density_d> components{1. * tesla, 2. * tesla,
                                                       3. * tesla};
    Vector<magnetic_flux_density_d> v1(rootCS, components);

    double const angle = 90. / 180. * M_PI;

    CoordinateSystem rotated1 = rootCS.rotate(zAxis, angle);
    CoordinateSystem rotated2 = rotated1.rotate(yAxis, angle);
    CoordinateSystem rotated3 = rotated2.rotate(zAxis, -angle);

    CoordinateSystem combined = rootCS.rotate(xAxis, -angle);

    auto comp1 = v1.GetComponents(rotated3);
    auto comp3 = v1.GetComponents(combined);
    REQUIRE((comp1 - comp3).norm().magnitude() == Approx(0).margin(absMargin));
  }
}

TEST_CASE("Sphere") {
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  Point center(rootCS, {0_m, 3_m, 4_m});
  Sphere sphere(center, 5_m);

  SECTION("GetCenter") {
    CHECK((sphere.GetCenter().GetCoordinates(rootCS) -
           QuantityVector<length_d>(0_m, 3_m, 4_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));
    CHECK(sphere.GetRadius() / 5_m == Approx(1));
  }

  SECTION("Contains") {
    REQUIRE_FALSE(sphere.Contains(Point(rootCS, {100_m, 0_m, 0_m})));
    REQUIRE(sphere.Contains(Point(rootCS, {2_m, 3_m, 4_m})));
  }
}

TEST_CASE("Trajectories") {
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  Point r0(rootCS, {0_m, 0_m, 0_m});

  SECTION("Line") {
    Vector<SpeedType::dimension_type> v0(rootCS,
                                         {3_m / second, 0_m / second, 0_m / second});

    Line const line(r0, v0);
    CHECK(
        (line.GetPosition(2_s).GetCoordinates() - QuantityVector<length_d>(6_m, 0_m, 0_m))
            .norm()
            .magnitude() == Approx(0).margin(absMargin));

    CHECK((line.PositionFromArclength(4_m).GetCoordinates() -
           QuantityVector<length_d>(4_m, 0_m, 0_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((line.GetPosition(7_s) - line.PositionFromArclength(line.ArcLength(0_s, 7_s)))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    auto const t = 1_s;
    Trajectory<Line> base(line, t);
    CHECK(line.GetPosition(t).GetCoordinates() == base.GetPosition(1.).GetCoordinates());

    CHECK(base.ArcLength(1_s, 2_s) / 1_m == Approx(3));

    CHECK((base.NormalizedDirection().GetComponents(rootCS) -
           QuantityVector<dimensionless_d>{1, 0, 0})
              .eVector.norm() == Approx(0).margin(absMargin));
  }

  SECTION("Helix") {
    Vector<SpeedType::dimension_type> const vPar(
        rootCS, {0_m / second, 0_m / second, 4_m / second});

    Vector<SpeedType::dimension_type> const vPerp(
        rootCS, {3_m / second, 0_m / second, 0_m / second});

    auto const T = 1_s;
    auto const omegaC = 2 * M_PI / T;

    Helix const helix(r0, omegaC, vPar, vPerp);

    CHECK((helix.GetPosition(1_s).GetCoordinates() -
           QuantityVector<length_d>(0_m, 0_m, 4_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((helix.GetPosition(0.25_s).GetCoordinates() -
           QuantityVector<length_d>(-3_m / (2 * M_PI), -3_m / (2 * M_PI), 1_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK(
        (helix.GetPosition(7_s) - helix.PositionFromArclength(helix.ArcLength(0_s, 7_s)))
            .norm()
            .magnitude() == Approx(0).margin(absMargin));

    auto const t = 1234_s;
    Trajectory<Helix> const base(helix, t);
    CHECK(helix.GetPosition(t).GetCoordinates() == base.GetPosition(1.).GetCoordinates());

    CHECK(base.ArcLength(0_s, 1_s) / 1_m == Approx(5));
  }
}
