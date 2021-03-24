/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#include <catch2/catch.hpp>

#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/RadioDetector.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

#include <vector>
#include <xtensor/xtensor.hpp>
#include <xtensor/xbuilder.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xcsv.hpp>
#include <istream>
#include <fstream>
#include <iostream>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>


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
#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/media/UniformMagneticField.hpp>



using namespace corsika;

double constexpr absMargin = 1.0e-7;

template <typename TInterface>
using MyExtraEnv =
UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;


TEST_CASE("Radio", "[processes]") {

  SECTION("CoREAS process") {

    // TODO: construct sychnotron radiation example with one electron

//     // Environment 1 (works)
//      // first step is to construct an environment for the propagation (uniform index 1)
//    using UniRIndex =
//    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType envCoREAS;
//
//    // get a coordinate system
//    const CoordinateSystemPtr rootCSCoREAS = envCoREAS.getCoordinateSystem();
//
//    auto MediumCoREAS = EnvType::createNode<Sphere>(
//        Point{rootCSCoREAS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const propsCoREAS = MediumCoREAS->setModelProperties<UniRIndex>(
//        1.000327, 1_kg / (1_m * 1_m * 1_m),
//        NuclearComposition(
//            std::vector<Code>{Code::Nitrogen},
//            std::vector<float>{1.f}));
//
//    envCoREAS.getUniverse()->addChild(std::move(MediumCoREAS));


    //////////////////////////////////////////////////////////////////////////////////////
//    // Environment 2 (works)
//    using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
//    using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
//        <IModelInterface>>>>;
//    using EnvType = Environment<AtmModel>;
//    EnvType envCoREAS;
//    CoordinateSystemPtr const& rootCSCoREAS = envCoREAS.getCoordinateSystem();
//    // get the center point
//    Point const center{rootCSCoREAS, 0_m, 0_m, 0_m};
//    // a refractive index
//    const double ri_{1.000327};
//
//    // the constant density
//    const auto density{19.2_g / cube(1_cm)};
//
//    // the composition we use for the homogeneous medium
//    NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
//                                               std::vector<float>{1.f});
//
//    // create magnetic field vector
//    Vector B1(rootCSCoREAS, 0_T, 0_T, 1_T);
//
//    auto Medium = EnvType::createNode<Sphere>(
//        center, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1, density, protonComposition);
//    envCoREAS.getUniverse()->addChild(std::move(Medium));


    //////////////////////////////////////////////////////////////////////////////////////
    // Environment 3 (works)
    using EnvironmentInterface =
    IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using EnvType = Environment<EnvironmentInterface>;
    EnvType envCoREAS;
    CoordinateSystemPtr const& rootCSCoREAS = envCoREAS.getCoordinateSystem();
    Point const center{rootCSCoREAS, 0_m, 0_m, 0_m};
    auto builder = make_layered_spherical_atmosphere_builder<
        EnvironmentInterface, MyExtraEnv>::create(center,
                                                  constants::EarthRadius::Mean, 1.000327,
                                                  Medium::AirDry1Atm,
                                                  MagneticFieldVector{rootCSCoREAS, 0_T,
                                                                      50_uT, 0_T});

    builder.setNuclearComposition(
        {{Code::Nitrogen, Code::Oxygen},
         {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

//    builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
//    builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
//    builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
//    builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
    builder.addLinearLayer(1e9_cm, 112.8_km);
    builder.assemble(envCoREAS);
//////////////////////////////////////////////////////////////////////////////////////////


    // now create antennas and detectors
    // the antennas location
    const auto point1{Point(envCoREAS.getCoordinateSystem(), 100_m, 2_m, 3_m)};
    const auto point2{Point(envCoREAS.getCoordinateSystem(), 4_m, 80_m, 6_m)};
    const auto point3{Point(envCoREAS.getCoordinateSystem(), 7_m, 8_m, 9_m)};
    const auto point4{Point(envCoREAS.getCoordinateSystem(), 5_m, 5_m, 10_m)};


    // create times for the antenna
    const TimeType t1{0_s}; // TODO: initialization of times to antennas! particle hits the observation level should be zero
    const TimeType t2{100_s};
    const InverseTimeType t3{1/1_s};
    const TimeType t4{11_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_name", point1, t1, t2, t3);
    TimeDomainAntenna ant2("antenna_name2", point2, t1, t2, t3);
//    TimeDomainAntenna ant3("antenna1", point1, 0_s, 2_s, 1/1e-7_s);


    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);
//    detector.addAntenna(ant3);



    // create a particle
    auto const particle{Code::Electron};
//    auto const particle{Code::Gamma};
    const auto pmass{get_mass(particle)};


    VelocityVector v0(rootCSCoREAS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCSCoREAS, 5_T, 5_T, 5_T);

    Line const line(point3, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 10_s;
    LeapFrogTrajectory base(point4, v0, B0, k, t);

    // create a new stack for each trial
    setup::Stack stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};

    // and create the momentum vector
    const auto plab{MomentumVector(rootCSCoREAS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCSCoREAS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(particle, E0, plab, pos, 0_ns))};

    auto const charge_ {get_charge(particle1.getPID())};
//    std::cout << "charge: " << charge_ << std::endl;
//    std::cout << "1 / c: " << 1. / constants::c << std::endl;

    // set up a track object
//      setup::Tracking tracking;

//    auto startPoint_ {base.getPosition(0)};
//    auto midPoint_ {base.getPosition(0.5)};
//    auto endPoint_ {base.getPosition(1)};
//    std::cout << "startPoint_: " << startPoint_ << std::endl;
//    std::cout << "midPoint_: " << midPoint_ << std::endl;
//    std::cout << "endPoint_: " << endPoint_ << std::endl;

//    auto velo_ {base.getVelocity(0)};
//    std::cout << "velocity: " << velo_ << std::endl;

//    auto startTime_ {particle1.getTime() - base.getDuration()}; // time at the start point of the track hopefully.
//    auto endTime_ {particle1.getTime()};
//    std::cout << "startTime_: " << startTime_ << std::endl;
//    std::cout << "endTime_: " << endTime_ << std::endl;
//
//    auto beta_ {((endPoint_ - startPoint_) / (constants::c * (endTime_ - startTime_))).normalized()};
//    std::cout << "beta_: " << beta_ << std::endl;

//    Vector<dimensionless_d> v1(rootCSCoREAS, {0, 0, 1});
//    std::cout << "v1: " << v1.getComponents() << std::endl;
//
//    std::cout << "beta_.dot(v1): " << beta_.dot(v1) << std::endl;
//
//    std::cout << "Pi: " << 1/M_PI << std::endl;
//
//    std::cout << "speed of light: " << constants::c << std::endl;
//
//    std::cout << "vacuum permitivity: " << constants::epsilonZero << std::endl;

    // Create a CoREAS instance
//    CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))> coreas1(detector, envCoREAS);

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector), CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))>, decltype(StraightPropagator(envCoREAS))>
        coreas(detector, envCoREAS);

    // check doContinuous and simulate methods
    coreas.doContinuous(particle1, base);
//    coreas1.simulate(particle1, base);

    // check writeOutput method -> should produce 2 csv files for each antenna
    coreas.writeOutput();
    }


//  SECTION("TimeDomainAntenna") {
//
//    // create an environment so we can get a coordinate system
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env6;
//
//    using UniRIndex =
//    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;
//
//
//    // the antenna location
//    const auto point1{Point(env6.getCoordinateSystem(), 1_m, 2_m, 3_m)};
//    const auto point2{Point(env6.getCoordinateSystem(), 4_m, 5_m, 6_m)};
//
//
//    // get a coordinate system
//    const CoordinateSystemPtr rootCS6 = env6.getCoordinateSystem();
//
//    auto Medium6 = EnvType::createNode<Sphere>(
//        Point{rootCS6, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const props6 = Medium6->setModelProperties<UniRIndex>(
//        1, 1_kg / (1_m * 1_m * 1_m),
//        NuclearComposition(
//            std::vector<Code>{Code::Nitrogen},
//            std::vector<float>{1.f}));
//
//    env6.getUniverse()->addChild(std::move(Medium6));
//
//    // create times for the antenna
//    const TimeType t1{10_s};
//    const TimeType t2{10_s};
//    const InverseTimeType t3{1/1_s};
//    const TimeType t4{11_s};
//
//    // check that I can create an antenna at (1, 2, 3)
//    TimeDomainAntenna ant1("antenna_name", point1, t1, t2, t3);
//    TimeDomainAntenna ant2("antenna_name2", point2, t4, t2, t3);
//
//    // assert that the antenna name is correct
//    REQUIRE(ant1.getName() == "antenna_name");
//    REQUIRE(ant2.getName() == "antenna_name2");
//
//    // and check that the antenna is at the right location
//    REQUIRE((ant1.getLocation() - point1).getNorm() < 1e-12 * 1_m);
//    REQUIRE((ant2.getLocation() - point2).getNorm() < 1e-12 * 1_m);
//
//    // construct a radio detector instance to store our antennas
//    AntennaCollection<TimeDomainAntenna> detector;
//
//    // add this antenna to the process
//    detector.addAntenna(ant1);
//    detector.addAntenna(ant2);
//    CHECK(detector.size() == 2);
//
//    // get a unit vector
//    Vector<dimensionless_d> v1(rootCS6, {0, 0, 1});
//    QuantityVector<ElectricFieldType::dimension_type> v11{10_V / 1_m, 10_V / 1_m, 10_V / 1_m};
//
//    Vector<dimensionless_d> v2(rootCS6, {0, 1, 0});
//    QuantityVector<ElectricFieldType::dimension_type> v22{20_V / 1_m, 20_V / 1_m, 20_V / 1_m};
//
//    // use receive methods
//    ant1.receive(15_s, v1, v11);
//    ant2.receive(16_s, v2, v22);
//
//    // use getWaveform() method
//    auto [t111, E1] = ant1.getWaveform();
//    CHECK(E1(5,0) - 10 == 0);
//    auto [t222, E2] = ant2.getWaveform();
//    CHECK(E2(5,0) -20 == 0);
//
//    // use the receive method in a for loop. It works now!
//    for (auto& xx : detector.getAntennas()) {
//      xx.receive(15_s, v1, v11);
//    }
//
//    // t & E are correct! uncomment to see them
////    for (auto& xx : detector.getAntennas()) {
////      auto [t,E] = xx.getWaveform();
////      std::cout << t << std::endl;
////      std::cout << " ... "<< std::endl;
////      std::cout << E << std::endl;
////      std::cout << " ... "<< std::endl;
////    }
//
//    // check output files. It works, uncomment to see.
////    int i = 1;
////    for (auto& antenna : detector.getAntennas()) {
////
////      auto [t,E] = antenna.getWaveform();
////      auto c0 = xt::hstack(xt::xtuple(t,E));
////      std::ofstream out_file ("antenna" + to_string(i) + "_output.csv");
////      xt::dump_csv(out_file, c0);
////      out_file.close();
////      ++i;
////
////    }
//
//    // check reset method for antennas. Uncomment to see they are zero
////    ant1.reset();
////    ant2.reset();
////
////    std::cout << ant1.waveformE_ << std::endl;
////    std::cout << ant2.waveformE_ << std::endl;
////
////    std::cout << " ... "<< std::endl;
////    std::cout << " ... "<< std::endl;
//
//    // check reset method for antenna collection. Uncomment to see they are zero
////    detector.reset();
////    for (auto& xx : detector.getAntennas()) {
////      std::cout << xx.waveformE_ << std::endl;
////      std::cout << " ... "<< std::endl;
////    }
//
//
//  }
//
//  // check that I can create working Straight Propagators in different environments
//  SECTION("Straight Propagator w/ Uniform Refractive Index") {
//
//    // create an environment with uniform refractive index of 1
//    using UniRIndex =
//        UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env;
//
//    // get a coordinate system
//    const CoordinateSystemPtr rootCS = env.getCoordinateSystem();
//
//    auto Medium = EnvType::createNode<Sphere>(
//        Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const props = Medium->setModelProperties<UniRIndex>(
//        1, 1_kg / (1_m * 1_m * 1_m),
//        NuclearComposition(
//            std::vector<Code>{Code::Nitrogen},
//            std::vector<float>{1.f}));
//
//    env.getUniverse()->addChild(std::move(Medium));
//
//    // get some points
//    Point p0(rootCS, {0_m, 0_m, 0_m});
//    //    Point p1(rootCS, {0_m, 0_m, 1_m});
//    //    Point p2(rootCS, {0_m, 0_m, 2_m});
//    //    Point p3(rootCS, {0_m, 0_m, 3_m});
//    //    Point p4(rootCS, {0_m, 0_m, 4_m});
//    //    Point p5(rootCS, {0_m, 0_m, 5_m});
//    //    Point p6(rootCS, {0_m, 0_m, 6_m});
//    //    Point p7(rootCS, {0_m, 0_m, 7_m});
//    //    Point p8(rootCS, {0_m, 0_m, 8_m});
//    //    Point p9(rootCS, {0_m, 0_m, 9_m});
//    Point p10(rootCS, {0_m, 0_m, 10_m});
//
//    // get a unit vector
//    Vector<dimensionless_d> v1(rootCS, {0, 0, 1});
//
//    //    // get a geometrical path of points
//    //    Path P1({p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10});
//
//    // construct a Straight Propagator given the uniform refractive index environment
//    StraightPropagator SP(env);
//
//    // store the outcome of the Propagate method to paths_
//    auto const paths_ = SP.propagate(p0, p10, 1_m);
//
//    // perform checks to paths_ components
//    for (auto const& path : paths_) {
//      CHECK((path.total_time_ / 1_s) - ((34_m / (3 * constants::c)) / 1_s) ==
//            Approx(0).margin(absMargin));
//      CHECK(path.average_refractive_index_ == Approx(1));
//      CHECK(path.emit_.getComponents() == v1.getComponents());
//      CHECK(path.receive_.getComponents() == v1.getComponents());
//      CHECK(path.R_distance_ == 10_m);
//      //      CHECK(std::equal(P1.begin(), P1.end(), path.points_.begin(),[]
//      //      (Point a, Point b) { return (a - b).norm() / 1_m < 1e-5;}));
//      //TODO:THINK ABOUT THE POINTS IN THE SIGNALPATH.H
//
////      std::cout << "path.total_time_: " << path.total_time_ << std::endl;
////      std::cout << "path.average_refractive_index_: " << path.average_refractive_index_ << std::endl;
////      std::cout << "path.emit_: " << path.emit_.getComponents() << std::endl;
////      std::cout << "path.R_distance_: " << path.R_distance_ << std::endl;
//
//
//    }
//
//    CHECK(paths_.size() == 1);
//  }
//
//    SECTION("Straight Propagator w/ Exponential Refractive Index") {
//
////    logging::set_level(logging::level::info);
////    corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");
//
//     // create an environment with exponential refractive index (n_0 = 1 & lambda = 0)
//    using ExpoRIndex = ExponentialRefractiveIndex<HomogeneousMedium
//        <IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env1;
//
//    //get another coordinate system
//    const CoordinateSystemPtr rootCS1 = env1.getCoordinateSystem();
//
//    auto Medium1 = EnvType::createNode<Sphere>(
//        Point{rootCS1, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const props1 =
//        Medium1
//            ->setModelProperties<ExpoRIndex>( 1, 0 / 1_m,
//               1_kg / (1_m * 1_m * 1_m),
//               NuclearComposition(
//               std::vector<Code>{Code::Nitrogen},
//               std::vector<float>{1.f}));
//
//    env1.getUniverse()->addChild(std::move(Medium1));
//
//    // get some points
//    Point pp0(rootCS1, {0_m, 0_m, 0_m});
////    Point pp1(rootCS1, {0_m, 0_m, 1_m});
////    Point pp2(rootCS1, {0_m, 0_m, 2_m});
////    Point pp3(rootCS1, {0_m, 0_m, 3_m});
////    Point pp4(rootCS1, {0_m, 0_m, 4_m});
////    Point pp5(rootCS1, {0_m, 0_m, 5_m});
////    Point pp6(rootCS1, {0_m, 0_m, 6_m});
////    Point pp7(rootCS1, {0_m, 0_m, 7_m});
////    Point pp8(rootCS1, {0_m, 0_m, 8_m});
////    Point pp9(rootCS1, {0_m, 0_m, 9_m});
//    Point pp10(rootCS1, {0_m, 0_m, 10_m});
//
//    // get a unit vector
//    Vector<dimensionless_d> vv1(rootCS1, {0, 0, 1});
//
//    // construct a Straight Propagator given the exponential refractive index environment
//    StraightPropagator SP1(env1);
//
//    // store the outcome of Propagate method to paths1_
//    auto const paths1_ = SP1.propagate(pp0, pp10, 1_m);
//
//    // perform checks to paths1_ components (this is just a sketch for now)
//    for (auto const& path :paths1_) {
//      CHECK( (path.total_time_ / 1_s)  - ((34_m / (3 * constants::c)) / 1_s)
//             == Approx(0).margin(absMargin) );
//      CHECK( path.average_refractive_index_ == Approx(1) );
//      CHECK( path.emit_.getComponents() == vv1.getComponents() );
//      CHECK( path.receive_.getComponents() == vv1.getComponents() );
//      CHECK( path.R_distance_ == 10_m );
//    }
//
//    CHECK( paths1_.size() == 1 );
//
//    /*
//    * A second environment with another exponential refractive index
//    */
//
//    // create an environment with exponential refractive index (n_0 = 2 & lambda = 2)
//    using ExpoRIndex = ExponentialRefractiveIndex<HomogeneousMedium
//        <IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env2;
//
//    //get another coordinate system
//    const CoordinateSystemPtr rootCS2 = env2.getCoordinateSystem();
//
//    auto Medium2 = EnvType::createNode<Sphere>(
//        Point{rootCS2, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const props2 =
//        Medium2
//            ->setModelProperties<ExpoRIndex>( 2, 2 / 1_m,
//                                              1_kg / (1_m * 1_m * 1_m),
//                                              NuclearComposition(
//                                                  std::vector<Code>{Code::Nitrogen},
//                                                  std::vector<float>{1.f}));
//
//    env2.getUniverse()->addChild(std::move(Medium2));
//
//    // get some points
//    Point ppp0(rootCS2, {0_m, 0_m, 0_m});
//    Point ppp10(rootCS2, {0_m, 0_m, 10_m});
//
//    // get a unit vector
//    Vector<dimensionless_d> vvv1(rootCS2, {0, 0, 1});
//
//    // construct a Straight Propagator given the exponential refractive index environment
//    StraightPropagator SP2(env2);
//
//    // store the outcome of Propagate method to paths1_
//    auto const paths2_ = SP2.propagate(ppp0, ppp10, 1_m);
//
//    // perform checks to paths1_ components (this is just a sketch for now)
//    for (auto const& path :paths2_) {
//      CHECK( (path.total_time_ / 1_s)  - ((3.177511688_m / (3 * constants::c)) / 1_s)
//             == Approx(0).margin(absMargin) );
//      CHECK( path.average_refractive_index_ == Approx(0.210275935) );
//      CHECK( path.emit_.getComponents() == vvv1.getComponents() );
//      CHECK( path.receive_.getComponents() == vvv1.getComponents() );
//      CHECK( path.R_distance_ == 10_m );
//    }
//
//    CHECK( paths2_.size() == 1 );
//
//    }

} // END: TEST_CASE("Radio", "[processes]")
