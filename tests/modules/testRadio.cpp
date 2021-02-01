/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#include <catch2/catch.hpp>

#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/TimeDomainDetector.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>


#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IRefractiveIndexModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ExponentialRefractiveIndex.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>

using namespace corsika;

double constexpr absMargin = 1.0e-7;

TEST_CASE("Radio", "[processes]") {

//  logging::set_level(logging::level::info);
//  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");


  // check that I can create and reset a TimeDomain process
  SECTION("TimeDomainDetector") {

    // construct a time domain detector
    TimeDomainDetector<TimeDomainAntenna> detector;

    //   // and construct some time domain radio process
    //   TimeDomain<ZHS<>, TimeDomainDetector<TimeDomainAntenna>> process(detector);
    //   TimeDomain<ZHS<CPU>, TimeDomainDetector<TimeDomainAntenna>> process2(detector);
    //   TimeDomain<ZHS<CPU>, decltype(detector)> process3(detector);
  }

  // check that I can create time domain antennas
  SECTION("TimeDomainDetector") {

    // create an environment so we can get a coordinate system
    Environment<IMediumModel> env;

    // the antenna location
    const auto point{Point(env.getCoordinateSystem(), 1_m, 2_m, 3_m)};

    // check that I can create an antenna at (1, 2, 3)
    const auto ant{TimeDomainAntenna("antenna_name", point)};

    // assert that the antenna name is correct
    REQUIRE(ant.getName() == "antenna_name");

    // and check that the antenna is at the right location
    REQUIRE((ant.getLocation() - point).getNorm() < 1e-12 * 1_m);

    // construct a radio detector instance to store our antennas
    TimeDomainDetector<TimeDomainAntenna> detector;

    // add this antenna to the process
    detector.addAntenna(ant);
  }

  // check that I can create working Straight Propagators in different environments
  SECTION("Straight Propagator w/ Uniform Refractive Index") {

    // create an environment with uniform refractive index of 1
    using UniRIndex =
        UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env;

    // get a coordinate system
    const CoordinateSystemPtr rootCS = env.getCoordinateSystem();

    auto Medium = EnvType::createNode<Sphere>(
        Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props = Medium->setModelProperties<UniRIndex>(
        1, 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition(
            std::vector<Code>{Code::Nitrogen},
            std::vector<float>{1.f}));

    env.getUniverse()->addChild(std::move(Medium));

    // get some points
    Point p0(rootCS, {0_m, 0_m, 0_m});
    //    Point p1(rootCS, {0_m, 0_m, 1_m});
    //    Point p2(rootCS, {0_m, 0_m, 2_m});
    //    Point p3(rootCS, {0_m, 0_m, 3_m});
    //    Point p4(rootCS, {0_m, 0_m, 4_m});
    //    Point p5(rootCS, {0_m, 0_m, 5_m});
    //    Point p6(rootCS, {0_m, 0_m, 6_m});
    //    Point p7(rootCS, {0_m, 0_m, 7_m});
    //    Point p8(rootCS, {0_m, 0_m, 8_m});
    //    Point p9(rootCS, {0_m, 0_m, 9_m});
    Point p10(rootCS, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> v1(rootCS, {0, 0, 1});

    //    // get a geometrical path of points
    //    Path P1({p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10});

    // construct a Straight Propagator given the uniform refractive index environment
    StraightPropagator SP(env);

    // store the outcome of the Propagate method to paths_
    auto const paths_ = SP.propagate(p0, p10, 1_m);

    // perform checks to paths_ components
    for (auto const& path : paths_) {
      CHECK((path.time_ / 1_s) - ((34_m / (3 * constants::c)) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractivity_ == Approx(1));
      CHECK(path.emit_.getComponents() == v1.getComponents());
      CHECK(path.receive_.getComponents() == v1.getComponents());
      //      CHECK(std::equal(P1.begin(), P1.end(), path.points_.begin(),[]
      //      (Point a, Point b) { return (a - b).norm() / 1_m < 1e-5;}));
      //TODO:THINK ABOUT THE POINTS IN THE SIGNALPATH.H
    }

    CHECK(paths_.size() == 1);
  }

    SECTION("Straight Propagator w/ Exponential Refractive Index") {

//    logging::set_level(logging::level::info);
//    corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

     // create an environment with exponential refractive index (n_0 = 1 & lambda = 0)
    using ExpoRIndex = ExponentialRefractiveIndex<HomogeneousMedium
        <IRefractiveIndexModel<IMediumModel>>>;

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env1;

    //get another coordinate system
    const CoordinateSystemPtr rootCS1 = env1.getCoordinateSystem();

    auto Medium1 = EnvType::createNode<Sphere>(
        Point{rootCS1, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props1 =
        Medium1
            ->setModelProperties<ExpoRIndex>( 1, 0 / 1_m,
               1_kg / (1_m * 1_m * 1_m),
               NuclearComposition(
               std::vector<Code>{Code::Nitrogen},
               std::vector<float>{1.f}));

    env1.getUniverse()->addChild(std::move(Medium1));

    // get some points
    Point pp0(rootCS1, {0_m, 0_m, 0_m});
//    Point pp1(rootCS1, {0_m, 0_m, 1_m});
//    Point pp2(rootCS1, {0_m, 0_m, 2_m});
//    Point pp3(rootCS1, {0_m, 0_m, 3_m});
//    Point pp4(rootCS1, {0_m, 0_m, 4_m});
//    Point pp5(rootCS1, {0_m, 0_m, 5_m});
//    Point pp6(rootCS1, {0_m, 0_m, 6_m});
//    Point pp7(rootCS1, {0_m, 0_m, 7_m});
//    Point pp8(rootCS1, {0_m, 0_m, 8_m});
//    Point pp9(rootCS1, {0_m, 0_m, 9_m});
    Point pp10(rootCS1, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> vv1(rootCS1, {0, 0, 1});

    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator SP1(env1);

    // store the outcome of Propagate method to paths1_
    auto const paths1_ = SP1.propagate(pp0, pp10, 1_m);

    // perform checks to paths1_ components (this is just a sketch for now)
    for (auto const& path :paths1_) {
      CHECK( (path.time_ / 1_s)  - ((34_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractivity_ == Approx(1) );
      CHECK( path.emit_.getComponents() == vv1.getComponents() );
      CHECK( path.receive_.getComponents() == vv1.getComponents() );
    }

    CHECK( paths1_.size() == 1 );

    /*
    * A second environment with another exponential refractive index
    */

    // create an environment with exponential refractive index (n_0 = 2 & lambda = 2)
    using ExpoRIndex = ExponentialRefractiveIndex<HomogeneousMedium
        <IRefractiveIndexModel<IMediumModel>>>;

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env2;

    //get another coordinate system
    const CoordinateSystemPtr rootCS2 = env2.getCoordinateSystem();

    auto Medium2 = EnvType::createNode<Sphere>(
        Point{rootCS2, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props2 =
        Medium2
            ->setModelProperties<ExpoRIndex>( 2, 2 / 1_m,
                                              1_kg / (1_m * 1_m * 1_m),
                                              NuclearComposition(
                                                  std::vector<Code>{Code::Nitrogen},
                                                  std::vector<float>{1.f}));

    env2.getUniverse()->addChild(std::move(Medium2));

    // get some points
    Point ppp0(rootCS2, {0_m, 0_m, 0_m});
    Point ppp10(rootCS2, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> vvv1(rootCS2, {0, 0, 1});

    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator SP2(env2);

    // store the outcome of Propagate method to paths1_
    auto const paths2_ = SP2.propagate(ppp0, ppp10, 1_m);

    // perform checks to paths1_ components (this is just a sketch for now)
    for (auto const& path :paths2_) {
      CHECK( (path.time_ / 1_s)  - ((3.177511688_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractivity_ == Approx(0.210275935) );
      CHECK( path.emit_.getComponents() == vvv1.getComponents() );
      CHECK( path.receive_.getComponents() == vvv1.getComponents() );
    }

    CHECK( paths2_.size() == 1 );

    }

//    SECTION("Construct a ZHS process.") {
//
//        // TODO: construct the environment for the propagator
//
//        // here we just use a deque to store the antenna
//        std::deque<TimeDomainAntenna> antennas;
//
//        // TODO: add time domain antennas to the deque
//
//        // and now create ZHS with this antenna collection
//        // and with a straight propagator
//        ZHS<decltype(antennas), StraightPropagator> zhs(antennas, env);
//        // TODO: do we need the explicit type declaration?
//
//        // here is where the simulation happens
//
//        // and call zhs.writeOutput() at the end.
//    }

} // END: TEST_CASE("Radio", "[processes]")
