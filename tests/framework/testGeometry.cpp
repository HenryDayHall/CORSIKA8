/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <cmath>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Helix.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Path.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/StraightTrajectory.hpp>
#include <corsika/framework/geometry/LeapFrogTrajectory.hpp>

#include <PhysicalUnitsCatch2.hpp> // namespace corsika::testing

using namespace corsika;
using namespace corsika::testing;

double constexpr absMargin = 1.0e-8;

TEST_CASE("Geometry CoordinateSystems") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] %v");

  CoordinateSystemPtr rootCS = get_root_CoordinateSystem();

  QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
  Point p1(rootCS, coordinates);
  CORSIKA_LOG_INFO("Point p1={}", p1);

  QuantityVector<magnetic_flux_density_d> components{1. * tesla, 0. * tesla, 0. * tesla};
  Vector<magnetic_flux_density_d> v1(rootCS, components);
  CORSIKA_LOG_INFO("Vector<magnetic_flux_density_d> v1={}", v1);

  CHECK((p1.getCoordinates() - coordinates).getNorm().magnitude() ==
        Approx(0).margin(absMargin));
  CHECK((p1.getCoordinates(rootCS) - coordinates).getNorm().magnitude() ==
        Approx(0).margin(absMargin));

  SECTION("translations") {
    QuantityVector<length_d> const translationVector{0_m, 4_m, 0_m};
    CORSIKA_LOG_INFO("QuantityVector<length_d> translationVector={}", translationVector);

    CoordinateSystemPtr translatedCS = make_translation(rootCS, translationVector);

    CHECK(translatedCS->getReferenceCS() == rootCS);

    CHECK((p1.getCoordinates(translatedCS) + translationVector).getNorm().magnitude() ==
          Approx(0).margin(absMargin));

    // Vectors are not subject to translations
    CHECK((v1.getComponents(rootCS) - v1.getComponents(translatedCS))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));

    Point p2(translatedCS, {0_m, 0_m, 0_m});
    CHECK(((p2 - p1).getComponents() - translationVector).getNorm().magnitude() ==
          Approx(0).margin(absMargin));
  }

  SECTION("multiple translations") {
    QuantityVector<length_d> const tv1{0_m, 5_m, 0_m};
    CoordinateSystemPtr cs2 = make_translation(rootCS, tv1);

    QuantityVector<length_d> const tv2{3_m, 0_m, 0_m};
    CoordinateSystemPtr cs3 = make_translation(rootCS, tv2);

    QuantityVector<length_d> const tv3{0_m, 0_m, 2_m};
    CoordinateSystemPtr cs4 = make_translation(cs3, tv3);

    CHECK(cs4->getReferenceCS()->getReferenceCS() == rootCS);

    CHECK(get_transformation(*cs3.get(), *cs2.get())
              .isApprox(make_translation(rootCS, {3_m, -5_m, 0_m})->getTransform()));
    CHECK(get_transformation(*cs2.get(), *cs3.get())
              .isApprox(make_translation(rootCS, {-3_m, +5_m, 0_m})->getTransform()));
  }

  SECTION("rotations") {
    QuantityVector<length_d> const axis{0_m, 0_m, 1_km};
    double const angle = 90. / 180. * M_PI;

    CoordinateSystemPtr rotatedCS = make_rotation(rootCS, axis, angle);
    CHECK(rotatedCS->getReferenceCS() == rootCS);

    CHECK(v1.getComponents(rotatedCS)[1].magnitude() ==
          Approx((-1. * tesla).magnitude()));

    // vector norm invariant under rotation
    CHECK(v1.getComponents(rotatedCS).getNorm().magnitude() ==
          Approx(v1.getComponents(rootCS).getNorm().magnitude()));
  }

  SECTION("multiple rotations") {
    QuantityVector<length_d> const zAxis{0_m, 0_m, 1_km};
    QuantityVector<length_d> const yAxis{0_m, 7_nm, 0_m};
    QuantityVector<length_d> const xAxis{2_m, 0_nm, 0_m};

    QuantityVector<magnetic_flux_density_d> components{1. * tesla, 2. * tesla,
                                                       3. * tesla};
    Vector<magnetic_flux_density_d> v1(rootCS, components);

    double const angle = 90. / 180. * M_PI;

    CoordinateSystemPtr rotated1 = make_rotation(rootCS, zAxis, angle);
    CoordinateSystemPtr rotated2 = make_rotation(rotated1, yAxis, angle);
    CoordinateSystemPtr rotated3 = make_rotation(rotated2, zAxis, -angle);

    CoordinateSystemPtr combined = make_rotation(rootCS, xAxis, -angle);

    auto comp1 = v1.getComponents(rotated3);
    auto comp3 = v1.getComponents(combined);
    CHECK((comp1 - comp3).getNorm().magnitude() == Approx(0).margin(absMargin));
  }

  SECTION("RotateToZ positive") {
    Vector const v{rootCS, 0_m, 1_m, 1_m};
    auto const csPrime = make_rotationToZ(rootCS, v);
    Vector const zPrime{csPrime, 0_m, 0_m, 5_m};
    Vector const xPrime{csPrime, 5_m, 0_m, 0_m};
    Vector const yPrime{csPrime, 0_m, 5_m, 0_m};

    CHECK(xPrime.dot(v).magnitude() == Approx(0).margin(absMargin));
    CHECK(yPrime.dot(v).magnitude() == Approx(0).margin(absMargin));
    CHECK((zPrime.dot(v) / 1_m).magnitude() == Approx(5 * sqrt(2)));

    CHECK(zPrime.getComponents(rootCS)[1].magnitude() ==
          Approx(zPrime.getComponents(rootCS)[2].magnitude()));
    CHECK(zPrime.getComponents(rootCS)[0].magnitude() == Approx(0));

    CHECK(xPrime.getComponents(rootCS).getEigenVector().dot(
              yPrime.getComponents(rootCS).getEigenVector()) == Approx(0));
    CHECK(zPrime.getComponents(rootCS).getEigenVector().dot(
              xPrime.getComponents(rootCS).getEigenVector()) == Approx(0));
    CHECK(yPrime.getComponents(rootCS).getEigenVector().dot(
              zPrime.getComponents(rootCS).getEigenVector()) == Approx(0));

    CHECK(yPrime.getComponents(rootCS).getEigenVector().dot(
              yPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
    CHECK(xPrime.getComponents(rootCS).getEigenVector().dot(
              xPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
    CHECK(zPrime.getComponents(rootCS).getEigenVector().dot(
              zPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
  }

  SECTION("RotateToZ negative") {
    Vector const v{rootCS, 0_m, 0_m, -1_m};
    auto const csPrime = make_rotationToZ(rootCS, v);
    Vector const zPrime{csPrime, 0_m, 0_m, 5_m};
    Vector const xPrime{csPrime, 5_m, 0_m, 0_m};
    Vector const yPrime{csPrime, 0_m, 5_m, 0_m};

    CHECK(zPrime.dot(v).magnitude() > 0);
    CHECK(xPrime.getComponents(rootCS).getEigenVector().dot(
              v.getComponents().getEigenVector()) == Approx(0));
    CHECK(yPrime.getComponents(rootCS).getEigenVector().dot(
              v.getComponents().getEigenVector()) == Approx(0));

    CHECK(xPrime.getComponents(rootCS).getEigenVector().dot(
              yPrime.getComponents(rootCS).getEigenVector()) == Approx(0));
    CHECK(zPrime.getComponents(rootCS).getEigenVector().dot(
              xPrime.getComponents(rootCS).getEigenVector()) == Approx(0));
    CHECK(yPrime.getComponents(rootCS).getEigenVector().dot(
              zPrime.getComponents(rootCS).getEigenVector()) == Approx(0));

    CHECK(yPrime.getComponents(rootCS).getEigenVector().dot(
              yPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
    CHECK(xPrime.getComponents(rootCS).getEigenVector().dot(
              xPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
    CHECK(zPrime.getComponents(rootCS).getEigenVector().dot(
              zPrime.getComponents(rootCS).getEigenVector()) ==
          Approx((5_m * 5_m).magnitude()));
  }
}

TEST_CASE("Geometry CoordinateSystem-hirarchy") {

  CoordinateSystemPtr rootCS = get_root_CoordinateSystem();

  CHECK(get_transformation(*rootCS.get(), *rootCS.get())
            .isApprox(EigenTransform::Identity()));

  // define the root coordinate system
  CoordinateSystemPtr root = get_root_CoordinateSystem();
  Point const p1(root, {0_m, 0_m, 0_m}); // the origin of the root CS

  // root -> cs2
  CoordinateSystemPtr cs2 = make_translation(root, {0_m, 0_m, 1_m});
  Point const p2(cs2, {0_m, 0_m, -1_m});

  // root -> cs2 -> cs3
  CoordinateSystemPtr cs3 = make_translation(cs2, {0_m, 0_m, -1_m});
  Point const p3(cs3, {0_m, 0_m, 0_m});

  // root -> cs2 -> cs4
  CoordinateSystemPtr cs4 = make_translation(cs2, {0_m, 0_m, -1_m});
  Point const p4(cs4, {0_m, 0_m, 0_m});

  // root -> cs2 -> cs4 -> cs5
  CoordinateSystemPtr cs5 =
      make_rotation(cs4, QuantityVector<length_d>{1_m, 0_m, 0_m}, 90 * degree_angle);
  Point const p5(cs5, {0_m, 0_m, 0_m});

  // root -> cs6
  CoordinateSystemPtr cs6 =
      make_rotation(root, QuantityVector<length_d>{1_m, 0_m, 0_m}, 90 * degree_angle);
  Point const p6(cs6, {0_m, 0_m, 0_m}); // the origin of the root CS

  // all points should be on top of each other

  CHECK_FALSE(
      get_transformation(*root.get(), *cs2.get()).isApprox(EigenTransform::Identity()));
  CHECK(get_transformation(*root.get(), *cs3.get()).isApprox(EigenTransform::Identity()));
  CHECK(get_transformation(*root.get(), *cs4.get()).isApprox(EigenTransform::Identity()));
  CHECK(get_transformation(*cs5.get(), *cs6.get()).isApprox(EigenTransform::Identity()));

  CHECK((p1 - p2).getNorm().magnitude() == Approx(0).margin(absMargin));
  CHECK((p1 - p3).getNorm().magnitude() == Approx(0).margin(absMargin));
  CHECK((p1 - p4).getNorm().magnitude() == Approx(0).margin(absMargin));
  CHECK((p1 - p5).getNorm().magnitude() == Approx(0).margin(absMargin));
  CHECK((p1 - p6).getNorm().magnitude() == Approx(0).margin(absMargin));
}

TEST_CASE("Geometry Sphere") {
  CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();
  Point center(rootCS, {0_m, 3_m, 4_m});
  Sphere sphere(center, 5_m);

  SECTION("getCenter") {
    CHECK((sphere.getCenter().getCoordinates(rootCS) -
           QuantityVector<length_d>(0_m, 3_m, 4_m))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));
    CHECK(sphere.getRadius() / 5_m == Approx(1));
  }

  SECTION("isInside") {
    CHECK_FALSE(sphere.contains(Point(rootCS, {100_m, 0_m, 0_m})));
    CHECK(sphere.contains(Point(rootCS, {2_m, 3_m, 4_m})));
  }
}

TEST_CASE("Geometry Trajectories") {
  CoordinateSystemPtr rootCS = get_root_CoordinateSystem();
  Point r0(rootCS, {0_m, 0_m, 0_m});

  SECTION("Line") {
    VelocityVector v0(rootCS, {3_m / second, 0_m / second, 0_m / second});

    Line const line(r0, v0);
    CHECK(
        (line.getPosition(2_s).getCoordinates() - QuantityVector<length_d>(6_m, 0_m, 0_m))
            .getNorm()
            .magnitude() == Approx(0).margin(absMargin));

    CHECK((line.getPositionFromArclength(4_m).getCoordinates() -
           QuantityVector<length_d>(4_m, 0_m, 0_m))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((line.getPosition(7_s) -
           line.getPositionFromArclength(line.getArcLength(0_s, 7_s)))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));

    auto const t = 1_s;
    StraightTrajectory base(line, t);
    CHECK(line.getPosition(t).getCoordinates() == base.getPosition(1.).getCoordinates());

    CHECK((base.getDirection(0).getComponents(rootCS) -
           QuantityVector<dimensionless_d>{1, 0, 0})
              .getNorm() == Approx(0).margin(absMargin));
  }

  SECTION("Helix") {
    VelocityVector const vPar(rootCS, {0_m / second, 0_m / second, 4_m / second});

    VelocityVector const vPerp(rootCS, {3_m / second, 0_m / second, 0_m / second});

    auto const T = 1_s;
    auto const omegaC = 2 * M_PI / T;

    Helix const helix(r0, omegaC, vPar, vPerp);

    CHECK((helix.getPosition(1_s).getCoordinates() -
           QuantityVector<length_d>(0_m, 0_m, 4_m))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((helix.getPosition(0.25_s).getCoordinates() -
           QuantityVector<length_d>(-3_m / (2 * M_PI), -3_m / (2 * M_PI), 1_m))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));

    CHECK((helix.getPosition(7_s) -
           helix.getPositionFromArclength(helix.getArcLength(0_s, 7_s)))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));
  }
}

TEST_CASE("Point") {
  // define a known CS
  CoordinateSystemPtr root = get_root_CoordinateSystem();

  // define known points
  Point p1(root, {0_m, 0_m, 0_m});
  Point p2(root, {0_m, 0_m, 5_m});
  Point p3(root, {1_m, 0_m, 0_m});
  Point p4(root, {5_m, 0_m, 0_m});
  Point p5(root, {0_m, 4_m, 0_m});
  Point p6(root, {0_m, 5_m, 0_m});

  SECTION("Test distance_to() method")
  // check distance_to() method
  CHECK(p1.distance_to(p2) / 1_m == Approx(5));
  CHECK(p3.distance_to(p4) / 1_m == Approx(4));
  CHECK(p5.distance_to(p6) / 1_m == Approx(1));
}

TEST_CASE("Path") {
  // define a known CS
  CoordinateSystemPtr root = get_root_CoordinateSystem();

  // define known points
  Point p1(root, {0_m, 0_m, 0_m});
  Point p2(root, {0_m, 0_m, 1_m});
  Point p3(root, {0_m, 0_m, 2_m});
  Point p4(root, {0_m, 0_m, 3_m});
  Point p5(root, {0_m, 0_m, 4_m});
  // define paths
  Path P1(p1);
  Path P2({p1, p2});
  Path P3({p1, p2, p3});
  // define deque that include point(s)
  std::deque<Point> l1 = {p1};
  std::deque<Point> l2 = {p1, p2};
  std::deque<Point> l3 = {p1, p2, p3};

  // test the various path constructors
  SECTION("Test Constructors") {
    // check constructor for one point
    CHECK(std::equal(P1.begin(), P1.end(), l1.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    // check constructor for collection of points
    CHECK(std::equal(P3.begin(), P3.end(), l3.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
  }

  // test the length and access methods
  SECTION("Test GetLength() and modifications to Path") {
    P1.AddToEnd(p2);
    P2.RemoveFromEnd();
    // Check modifications to path
    CHECK(std::equal(P1.begin(), P1.end(), l2.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    CHECK(std::equal(P2.begin(), P2.end(), l1.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    // Check GetStart(), GetEnd(), GetPoint()
    CHECK((P3.GetEnd() - P3.GetStart()).getNorm() / 1_m == Approx(2));
    CHECK((P1.GetPoint(1) - p2).getNorm() / 1_m == Approx(0));
    // Check GetLength()
    CHECK(P1.GetLength() / 1_m == Approx(1));
    CHECK(P2.GetLength() / 1_m == Approx(0));
    CHECK(P3.GetLength() / 1_m == Approx(2));
    P2.RemoveFromEnd();
    CHECK(P2.GetLength() / 1_m == Approx(0)); // Check the length of an empty path
    P3.AddToEnd(p4);
    P3.AddToEnd(p5);
    CHECK(P3.GetLength() / 1_m == Approx(4));
    P3.RemoveFromEnd();
    CHECK(P3.GetLength() / 1_m == Approx(3)); // Check RemoveFromEnd() else case
    // Check GetNSegments()
    CHECK(P3.GetNSegments() - 3 == Approx(0));
  }
}