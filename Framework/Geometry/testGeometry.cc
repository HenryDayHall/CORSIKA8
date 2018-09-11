#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <Geometry/CoordinateSystem.h>
#include <Geometry/Point.h>
#include <Geometry/Sphere.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

using namespace phys::units;
using namespace phys::units::literals;

TEST_CASE("transformations between CoordinateSystems") {
  CoordinateSystem rootCS;

  REQUIRE(CoordinateSystem::GetTransformation(rootCS, rootCS)
              .isApprox(EigenTransform::Identity()));

  QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
  Point p1(rootCS, coordinates);

  QuantityVector<magnetic_flux_density_d> components{1. * tesla, 0. * tesla, 0. * tesla};
  Vector<magnetic_flux_density_d> v1(rootCS, components);

  REQUIRE((p1.GetCoordinates() - coordinates).norm().magnitude() == Approx(0));
  REQUIRE((p1.GetCoordinates(rootCS) - coordinates).norm().magnitude() == Approx(0));

  SECTION("unconnected CoordinateSystems") {
    CoordinateSystem rootCS2;
    REQUIRE_THROWS(CoordinateSystem::GetTransformation(rootCS, rootCS2));
  }

  SECTION("translations") {
    QuantityVector<length_d> const translationVector{0_m, 4_m, 0_m};

    CoordinateSystem translatedCS = rootCS.translate(translationVector);

    REQUIRE(translatedCS.GetReference() == &rootCS);

    REQUIRE((p1.GetCoordinates(translatedCS) + translationVector).norm().magnitude() ==
            Approx(0));

    // Vectors are not subject to translations
    REQUIRE(
        (v1.GetComponents(rootCS) - v1.GetComponents(translatedCS)).norm().magnitude() ==
        Approx(0));

    Point p2(translatedCS, {0_m, 0_m, 0_m});
    REQUIRE(((p2 - p1).GetComponents() - translationVector).norm().magnitude() ==
            Approx(0));
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

    double const angle = 90. / 180. * M_PI;

    CoordinateSystem rotated1 = rootCS.rotate(zAxis, angle);
    CoordinateSystem rotated2 = rotated1.rotate(yAxis, angle);
    CoordinateSystem rotated3 = rotated2.rotate(zAxis, -angle);

    CoordinateSystem combined = rootCS.rotate(xAxis, -angle);

    auto comp1 = v1.GetComponents(rootCS);
    auto comp3 = v1.GetComponents(combined);
    REQUIRE((comp1 - comp3).norm().magnitude() == Approx(0));
  }
}

TEST_CASE("Sphere") {
  CoordinateSystem rootCS;
  Point center(rootCS, {0_m, 3_m, 4_m});
  Sphere sphere(center, 5_m);

  SECTION("isInside") {
    REQUIRE_FALSE(sphere.isInside(Point(rootCS, {100_m, 0_m, 0_m})));

    REQUIRE(sphere.isInside(Point(rootCS, {2_m, 3_m, 4_m})));
  }
}
