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
#include <corsika/framework/geometry/Box.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Helix.hpp>
#include <corsika/framework/geometry/LeapFrogTrajectory.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Path.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/StraightTrajectory.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <PhysicalUnitsCatch2.hpp> // namespace corsike::testing

using namespace corsika;
using namespace corsika::testing;

double constexpr absMargin = 1.0e-8;

TEST_CASE("Geometry CoordinateSystems") {

  logging::set_level(logging::level::info);

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

  SECTION("basic operations") {
    auto testV0 = v1 * 6;
    CHECK(testV0.getNorm() / tesla == Approx(6));
    auto testV1 = 6 * v1;
    CHECK(testV1.getNorm() / tesla == Approx(6));
    auto testV2 = 6_m * v1;
    CHECK(testV2.getNorm() / (tesla * meter) == Approx(6));
    auto testV3 = v1 * 6_m;
    CHECK(testV3.getNorm() / (tesla * meter) == Approx(6));
  }

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
    CHECK(p2.getX(rootCS) == 0_m);
    CHECK(p2.getY(rootCS) == 4_m);
    CHECK(p2.getZ(rootCS) == 0_m);
    CHECK(p2.getX(translatedCS) == 0_m);
    CHECK(p2.getY(translatedCS) == 0_m);
    CHECK(p2.getZ(translatedCS) == 0_m);

    Vector<magnetic_flux_density_d> v2(translatedCS, components);
    CHECK((v2 - v1).getNorm() / 1_T == Approx(0));
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

    // this is not possible
    QuantityVector<length_d> const axis_invalid{0_m, 0_m, 0_km};
    CHECK_THROWS(make_rotation(rootCS, axis_invalid, angle));
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

  CHECK(p1.getX(root) == 0_m);
  CHECK(p1.getY(root) == 0_m);
  CHECK(p1.getZ(root) == 0_m);

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

TEST_CASE("Geometry Box") {
  CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();
  auto initialCS = make_translation(rootCS, {0_m, 0_m, 5_m});
  Point center = Point(initialCS, {0_m, 0_m, 0_m});
  Box box(initialCS, 4_m, 5_m, 6_m);

  SECTION("getCenter") {
    CHECK((box.getCenter().getCoordinates(rootCS) - center.getCoordinates(rootCS))
              .getNorm()
              .magnitude() == Approx(0).margin(absMargin));
    CHECK(box.getX() / 4_m == Approx(1));
    CHECK(box.getY() / 5_m == Approx(1));
    CHECK(box.getZ() / 6_m == Approx(1));
  }

  SECTION("isInside") {
    CHECK_FALSE(box.contains(Point(rootCS, {4.5_m, 0_m, 0_m})));
    CHECK(box.contains(Point(rootCS, {0_m, 4.5_m, 0_m})));
  }

  SECTION("internal coordinate") {
    CoordinateSystemPtr const internalCS = box.getCoordinateSystem();
    auto coordinate = box.getCenter().getCoordinates(internalCS);
    CHECK(coordinate.getX() / 1_m == Approx(0));
    CHECK(coordinate.getY() / 1_m == Approx(0));
    CHECK(coordinate.getZ() / 1_m == Approx(0));
  }

  SECTION("rotation") {
    QuantityVector<length_d> const axis_z{0_m, 0_m, 1_m};
    box.rotate(axis_z, 90);
    CHECK(box.contains(Point(rootCS, {4.5_m, 0_m, 0_m})));
    CHECK_FALSE(box.contains(Point(rootCS, {0_m, 4.5_m, 0_m})));
  }

  SECTION("from different coordinate") {
    QuantityVector<length_d> const axis_z{0_m, 0_m, 1_m};
    auto rotatedCS = make_rotation(initialCS, axis_z, 90);
    Point center(rootCS, {0_m, 0_m, 5_m});
    Box box(rotatedCS, 4_m, 5_m, 6_m);
    CHECK(box.contains(Point(rootCS, {4.5_m, 0_m, 0_m})));
    CHECK_FALSE(box.contains(Point(rootCS, {0_m, 4.5_m, 0_m})));
  }
}

TEST_CASE("Geometry Trajectories") {
  CoordinateSystemPtr rootCS = get_root_CoordinateSystem();
  Point r0(rootCS, {0_m, 0_m, 0_m});
  // Create a particle and and a stack so we can test .getTime() method
  const Code particle{Code::Electron};
  setup::Stack stack;
  // the mass of the particle
  const auto pmass{get_mass(particle)};
  // set an arbitrary energy value
  const HEPEnergyType E0{1_TeV};
  // compute the corresponding momentum
  const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
  // create the momentum vector
  const auto plab{MomentumVector(rootCS, {0_GeV, 0_GeV, P0})};
  // create an arbitrary location of the particle
  const Point pos(rootCS, 50_m, 10_m, 80_m);
  // add it finally to the stack
  auto const particle1{stack.addParticle(
      std::make_tuple(particle, calculate_kinetic_energy(plab.getNorm(), pmass),
                      plab.normalized(), pos, 0_ns))};

  SECTION("Line") {
    SpeedType const V0 = 3_m / second;
    VelocityVector v0(rootCS, {V0, 0_m / second, 0_m / second});

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

    CHECK((line.getTimeFromArclength(10_m) / (10_m / v0.getNorm()) == Approx(1)));

    auto const t = 1_s;
    StraightTrajectory base(line, t);
    CHECK(line.getPosition(t).getCoordinates() == base.getPosition(1.).getCoordinates());
    // test the getTime() method for straight trajectory
    CHECK(base.getTime(particle1, 1) / 1_s == Approx(1));

    CHECK((base.getDirection(0).getComponents(rootCS) -
           QuantityVector<dimensionless_d>{1, 0, 0})
              .getNorm() == Approx(0).margin(absMargin));

    base.setDuration(0_s);

    CHECK(base.getDuration() / 1_s == Approx(0));
    CHECK(base.getLength() / 1_m == Approx(0));

    base.setDuration(10_s);

    CHECK(base.getDuration() / 1_s == Approx(10));

    StraightTrajectory base2(line,
                             std::numeric_limits<TimeType::value_type>::infinity() * 1_s);
    base2.setDuration(10_s);
    CHECK(base2.getDuration() / 1_s == Approx(10));
    // test the getTime() method for straight trajectory
    CHECK(base2.getTime(particle1, 0) / 1_s == Approx(0));

    base2.setLength(1.3_m);
    CHECK(base2.getDuration() * V0 / meter == Approx(1.3));
    CHECK(base2.getLength() / meter == Approx(1.3));
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

  SECTION("LeapFrog Trajectory") {

    // Create a velocity Vector
    VelocityVector v0(rootCS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});
    // Create a magnetic filed Vector
    Vector B0(rootCS, 5_T, 5_T, 5_T);
    // provide a k constant and a random time for the LeapFrog Trajectory
    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};
    auto const t = 1e-12_s;

    // construct a LeapFrog Trajectory
    LeapFrogTrajectory base(pos, v0, B0, k, t);

    // test the getTime() method for trajectories
    CHECK((base.getTime(particle1, 1) - t) / 1_s == Approx(0));
    CHECK(base.getTime(particle1, 0) / 1_s == Approx(0));
    CHECK((base.getTime(particle1, 0) + t) / 1_s == Approx(1e-12));
  }
}

TEST_CASE("Distance between points") {
  // define a known CS
  CoordinateSystemPtr root = get_root_CoordinateSystem();

  // define known points
  Point p1(root, {0_m, 0_m, 0_m});
  Point p2(root, {0_m, 0_m, 5_m});
  Point p3(root, {1_m, 0_m, 0_m});
  Point p4(root, {5_m, 0_m, 0_m});
  Point p5(root, {0_m, 4_m, 0_m});
  Point p6(root, {0_m, 5_m, 0_m});

  // check distance() function
  CHECK(distance(p1, p2) / 1_m == Approx(5));
  CHECK(distance(p3, p4) / 1_m == Approx(4));
  CHECK(distance(p5, p6) / 1_m == Approx(1));
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
  SECTION("Test getLength() and modifications to Path") {
    P1.addToEnd(p2);
    P2.removeFromEnd();
    // Check modifications to path
    CHECK(std::equal(P1.begin(), P1.end(), l2.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    CHECK(std::equal(P2.begin(), P2.end(), l1.begin(),
                     [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    // Check GetStart(), GetEnd(), GetPoint()
    CHECK((P3.getEnd() - P3.getStart()).getNorm() / 1_m == Approx(2));
    CHECK((P1.getPoint(1) - p2).getNorm() / 1_m == Approx(0));
    // Check GetLength()
    CHECK(P1.getLength() / 1_m == Approx(1));
    CHECK(P2.getLength() / 1_m == Approx(0));
    CHECK(P3.getLength() / 1_m == Approx(2));
    P2.removeFromEnd();
    CHECK(P2.getLength() / 1_m == Approx(0)); // Check the length of an empty path
    P3.addToEnd(p4);
    P3.addToEnd(p5);
    CHECK(P3.getLength() / 1_m == Approx(4));
    P3.removeFromEnd();
    CHECK(P3.getLength() / 1_m == Approx(3)); // Check RemoveFromEnd() else case
    // Check GetNSegments()
    CHECK(P3.getNSegments() - 3 == Approx(0));
    P3.removeFromEnd();
    P3.removeFromEnd();
    P3.removeFromEnd();
    CHECK(P3.getNSegments() == Approx(0));
  }
}