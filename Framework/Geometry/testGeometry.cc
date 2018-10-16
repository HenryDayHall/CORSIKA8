
/**
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
#include <catch2/catch.hpp>

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/Helix.h>
#include <corsika/geometry/LineTrajectory.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

using namespace corsika::geometry;
using namespace corsika::units::si;

double constexpr absMargin = 1.0e-8;

TEST_CASE("transformations between CoordinateSystems") {
  CoordinateSystem rootCS = CoordinateSystem::CreateRootCS();

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

  SECTION("unconnected CoordinateSystems") {
    CoordinateSystem rootCS2 = CoordinateSystem::CreateRootCS();
    REQUIRE_THROWS(CoordinateSystem::GetTransformation(rootCS, rootCS2));
  }

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
  CoordinateSystem rootCS = CoordinateSystem::CreateRootCS();
  Point center(rootCS, {0_m, 3_m, 4_m});
  Sphere sphere(center, 5_m);

  SECTION("isInside") {
    REQUIRE_FALSE(sphere.Contains(Point(rootCS, {100_m, 0_m, 0_m})));
    REQUIRE(sphere.Contains(Point(rootCS, {2_m, 3_m, 4_m})));
  }
}

TEST_CASE("Trajectories") {
  CoordinateSystem rootCS = CoordinateSystem::CreateRootCS();
  Point r0(rootCS, {0_m, 0_m, 0_m});

  SECTION("Line") {
    Vector<SpeedType::dimension_type> v0(rootCS,
                                         {1_m / second, 0_m / second, 0_m / second});

    LineTrajectory const lineTrajectory(r0, v0);
    CHECK((lineTrajectory.GetPosition(2_s).GetCoordinates() -
           QuantityVector<length_d>(2_m, 0_m, 0_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    BaseTrajectory const* base = &lineTrajectory;
    CHECK(lineTrajectory.GetPosition(2_s).GetCoordinates() ==
          base->GetPosition(2_s).GetCoordinates());

    CHECK(base->DistanceBetween(1_s, 2_s) / 1_m == Approx(1));
  }

  SECTION("Helix") {
    Vector<SpeedType::dimension_type> const vPar(
        rootCS, {0_m / second, 0_m / second, 4_m / second});

    Vector<SpeedType::dimension_type> const vPerp(
        rootCS, {3_m / second, 0_m / second, 0_m / second});

    auto const omegaC = 2 * M_PI / 1_s;

    Helix const helix(r0, omegaC, vPar, vPerp);

    CHECK((helix.GetPosition(1_s).GetCoordinates() -
           QuantityVector<length_d>(0_m, 0_m, 4_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((helix.GetPosition(0.25_s).GetCoordinates() -
           QuantityVector<length_d>(-3_m / (2 * M_PI), -3_m / (2 * M_PI), 1_m))
              .norm()
              .magnitude() == Approx(0).margin(absMargin));

    BaseTrajectory const* base = &helix;
    CHECK(helix.GetPosition(1234_s).GetCoordinates() ==
          base->GetPosition(1234_s).GetCoordinates());

    CHECK(base->DistanceBetween(1_s, 2_s) / 1_m == Approx(5));
  }
}
