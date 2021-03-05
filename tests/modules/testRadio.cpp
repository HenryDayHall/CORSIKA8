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
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/media/UniformMagneticField.hpp>



using namespace corsika;

double constexpr absMargin = 1.0e-7;

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

template <typename T>
using UniRIndex =
UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<MediumPropertyModel<UniformMagneticField<T>>>>>;



TEST_CASE("Radio", "[processes]") {

  // create an environment with uniform refractive index of 1

  using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
  EnvType env9;


  using MyHomogeneousModel = MediumPropertyModel<
      UniformMagneticField<HomogeneousMedium<UniformRefractiveIndex<IRefractiveIndexModel<IMediumModel>>>>>;

  auto& universe = *(env9.getUniverse());
  CoordinateSystemPtr const& rootCS9 = env9.getCoordinateSystem();

  auto world = EnvType::createNode<Sphere>(Point{rootCS9, 0_m, 0_m, 0_m}, 150_km);

  world->setModelProperties<MyHomogeneousModel>(
      Medium::AirDry1Atm, MagneticFieldVector(rootCS9, 0_T, 0_T, 1_T),
      1_kg / (1_m * 1_m * 1_m),
      NuclearComposition(std::vector<Code>{Code::Hydrogen},
                         std::vector<float>{(float)1.}), 1);

  universe.addChild(std::move(world));

//    world->setModelProperties<UniRIndex>(
//        1);
//
//    universe.addChild(std::move(world));

  SECTION("TimeDomainAntenna") {

    // create an environment so we can get a coordinate system
    Environment<IMediumModel> env;

    // the antenna location
    const auto point1{Point(env.getCoordinateSystem(), 1_m, 2_m, 3_m)};
    const auto point2{Point(env.getCoordinateSystem(), 4_m, 5_m, 6_m)};

    // create times for the antenna
    const TimeType t1{10_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1/1_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_name", point1, t1, t2, t3);
    TimeDomainAntenna ant2("antenna_name2", point2, 11_s, t2, t3);

    // assert that the antenna name is correct
    REQUIRE(ant1.getName() == "antenna_name");

    // and check that the antenna is at the right location
    REQUIRE((ant1.getLocation() - point1).getNorm() < 1e-12 * 1_m);

    std::vector<TimeDomainAntenna> detector;
    detector.push_back(ant1);

//    // construct a radio detector instance to store our antennas
//    AntennaCollection<TimeDomainAntenna> detector;
//
//    // add this antenna to the process
//    detector.addAntenna(ant1);
////    detector.addAntenna(ant2);


//    EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    using UniRIndex =
//    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;

//    using UniRIndex =
//    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env;

//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env6;
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

//    using EnvType = setup::Environment;
//    EnvType env6;
//
//    CoordinateSystemPtr const& rootCS = env6.getCoordinateSystem();
//
//    Point const center{rootCS, 0_m, 0_m, 0_m};
//
//    auto builder = make_layered_spherical_atmosphere_builder<
//        setup::EnvironmentInterface, UniRIndex>::create(center,
//                                                         constants::EarthRadius::Mean,
//                                                         Medium::AirDry1Atm,
//                                                         1,
//                                                         MagneticFieldVector{rootCS, 0_T,
//                                                                             50_uT, 0_T});
//
//    builder.setNuclearComposition(
//        {{Code::Nitrogen, Code::Oxygen},
//         {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

//    builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
//    builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
//    builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
//    builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
//    builder.addLinearLayer(1e9_cm, 112.8_km);
//    builder.assemble(env6);

    // setup particle stack, and add primary particle
//    setup::Stack stack;
//    stack.clear();
//    const Code beamCode = Code::Nucleus;
//    unsigned short const A = std::stoi(std::string(argv[1]));
//    unsigned short Z = std::stoi(std::string(argv[2]));
//    auto const mass = get_nucleus_mass(A, Z);
//    const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[3]));
//    double theta = 0.;
//    auto const thetaRad = theta / 180. * M_PI;
//
//    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
//      return sqrt((Elab - m) * (Elab + m));
//    };
//    HEPMomentumType P0 = elab2plab(E0, mass);
//    auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
//      return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
//    };
//
//    auto const [px, py, pz] = momentumComponents(thetaRad, P0);
//    auto plab = MomentumVector(rootCS, {px, py, pz});
//    std::cout << "input particle: " << beamCode << std::endl;
//    std::cout << "input angles: theta=" << theta << std::endl;
//    std::cout << "input momentum: " << plab.getComponents() / 1_GeV
//         << ", norm = " << plab.getNorm() << std::endl;
//
//    auto const observationHeight = 0_km + builder.getEarthRadius();
//    auto const injectionHeight = 112.75_km + builder.getEarthRadius();
//    auto const t = -observationHeight * cos(thetaRad) +
//                   sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
//                        static_pow<2>(injectionHeight));
//    Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
//    Point const injectionPos =
//        showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;
//
//    std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;








//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType env6;
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

    // TODO: this is crucial to be solved I thought it was the getAntenna() method but nope.
    // Tried the same with a good old out of the box std::vector and still no luck
    // The problem should be in the constructor (?)
//    auto x = detector.getAntennas();
//    detector.push_back(ant1);

//    for (auto& xx : detector) {
//      auto [t1111, E1111] = xx.getWaveform();
//      CHECK(E1111(5,0) - 10 == 0);
//    }


//    // use getWaveform() method
//    auto [t11, E1] = ant1.getWaveform();
//    CHECK(E1(5,0) - 10 == 0);
//
//    auto [t222, E2] = ant2.getWaveform();
//    CHECK(E2(5,0) -20 == 0);


    // the rest is just random ideas
    //////////////////////////////////////////////////////////////////////////////////////

//    for (auto& kostas : detector.getAntennas()) {
//      std::cout << kostas.getName() << std::endl;
//      kostas.receive(15_s, v1, v11);
//      std::cout << kostas.waveformE_;
//      auto [t, E] = kostas.getWaveform();
//      std::cout << E(5,0) << std::endl;
//      kostas.receive(16_s, v2, v22);
//      std::cout << E(5,0) << std::endl;
//    }



    //    auto t33{static_cast<int>(t1 / t2)};
//    std::cout << t33 << std::endl;
//    xt::xtensor<double,2> waveformE_ = xt::zeros<double>({2, 3});
//    xt::xtensor<double,2> res = xt::view(waveformE_);
//    std::cout << ant1.waveformE_ << std::endl;
//    std::cout << " " << std::endl;
//    std::cout << ant2.waveformE_ << std::endl;
//    std::cout << " " << std::endl;
//    std::cout << ant1.times_ << std::endl;
//    std::cout << " " << std::endl;
//    std::cout << ant2.times_ << std::endl;
//   auto [t, E] = ant2.getWaveform();
//

//
//
//   std::ofstream out_file("antenna_output.csv");
//    xt::dump_csv(out_file, t);
//    xt::dump_csv(out_file, E);
//    out_file.close();


//    for (auto& antenna : detector.getAntennas()) {
//      auto [t, E] = antenna.getWaveform();
//    }

//    int i = 1;
//    auto x = detector.getAntennas();
//    for (auto& antenna : x) {
//
//      auto [t, E] = antenna.getWaveform();
//      std::ofstream out_file("antenna" + to_string(i) + "_output.csv");
//      std::cout << E(5,0) << std::endl;
//      xt::dump_csv(out_file, t);
//      xt::dump_csv(out_file, E);
//      out_file.close();
//      ++i;
//    }


//    detector.writeOutput();
//    ant1.reset();
//    ant2.reset();
//
//    std::cout << ant1.waveformE_ << std::endl;
//    std::cout << ant2.waveformE_ << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////
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
      CHECK((path.total_time_ / 1_s) - ((34_m / (3 * constants::c)) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.emit_.getComponents() == v1.getComponents());
      CHECK(path.receive_.getComponents() == v1.getComponents());
      CHECK(path.R_distance_ == 10_m);
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
      CHECK( (path.total_time_ / 1_s)  - ((34_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractive_index_ == Approx(1) );
      CHECK( path.emit_.getComponents() == vv1.getComponents() );
      CHECK( path.receive_.getComponents() == vv1.getComponents() );
      CHECK( path.R_distance_ == 10_m );
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
      CHECK( (path.total_time_ / 1_s)  - ((3.177511688_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractive_index_ == Approx(0.210275935) );
      CHECK( path.emit_.getComponents() == vvv1.getComponents() );
      CHECK( path.receive_.getComponents() == vvv1.getComponents() );
      CHECK( path.R_distance_ == 10_m );
    }

    CHECK( paths2_.size() == 1 );

    }

//    SECTION("ZHS process") {
//      // first step is to construct an environment for the propagation (uni index)
//    using UniRIndex =
//    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;
//
//    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
//    EnvType envZHS;
//
//    // get a coordinate system
//    const CoordinateSystemPtr rootCSzhs = envZHS.getCoordinateSystem();
//
//    auto MediumZHS = EnvType::createNode<Sphere>(
//        Point{rootCSzhs, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());
//
//    auto const propsZHS = MediumZHS->setModelProperties<UniRIndex>(
//        1, 1_kg / (1_m * 1_m * 1_m),
//        NuclearComposition(
//            std::vector<Code>{Code::Nitrogen},
//            std::vector<float>{1.f}));
//
//    envZHS.getUniverse()->addChild(std::move(MediumZHS));
//
//
//    // now create antennas and detectors
//    // the antennas location
//    const auto point1{Point(envZHS.getCoordinateSystem(), 1_m, 2_m, 3_m)};
//    const auto point2{Point(envZHS.getCoordinateSystem(), 4_m, 5_m, 6_m)};
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
//    // construct a radio detector instance to store our antennas
//    AntennaCollection<TimeDomainAntenna> detector;
//
//    // add this antenna to the process
//    detector.addAntenna(ant1);
//    detector.addAntenna(ant2);
//
//    // create a particle
//    auto const particle{Code::Electron};
//    const auto pmass{get_mass(particle)};
//
//    // create a new stack for each trial
//    setup::Stack stack;
//
//    // construct an energy
//    const HEPEnergyType E0{1_TeV};
//
//    // compute the necessary momentumn
//    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
//
//    // and create the momentum vector
//    const auto plab{MomentumVector(rootCSzhs, {0_GeV, 0_GeV, P0})};
//
//    // and create the location of the particle in this coordinate system
//    const Point pos(rootCSzhs, 50_m, 10_m, 80_m);
//
//    // add the particle to the stack
//    auto const particle1{stack.addParticle(std::make_tuple(particle, E0, plab, pos, 0_ns))};
//
//    // set up a track object
//      setup::Tracking tracking;
//
//    // Create a ZHS instance
//    ZHS<decltype(detector), decltype(StraightPropagator(envZHS))> zhs(detector, envZHS);
//
//    // call ZHS over the track
//    zhs.doContinuous(particle1, tracking);
//    zhs.simulate(particle1, tracking);
//
//    zhs.writeOutput();
//    }

//    SECTION("Construct a ZHS process.") {
//
//        // TODO: construct the environment for the propagator

    //////////////////////////////////////////////////////////////////////////////////////
//    // useful information
//  // create a TimeDomain process
//  auto zhs{TimeDomain<ZHS<CPU>, decltype(detector)>(detector)};
//
//  // get the antenna back from the process and check the properties
//  REQUIRE(detector.GetAntennas().cbegin()->GetName() == "antenna_name");
//
//  // and check the location is the same
//  REQUIRE((detector.GetAntennas().cbegin()->GetLocation() - point).norm() <
//          1e-12 * 1_m);
//  // and check that the number of antennas visible to the process has increased
//  REQUIRE(zhs.GetDetector().GetAntennas().size() == 2);

//////////////////////////////////////////////////////////////////////////////////////////

//  using VelocityVec = Vector<corsika::units::si::SpeedType::dimension_type>;
//
//  // setup environment, geometry
//  environment::Environment<environment::IMediumModel> env;
//
//  // and get the coordinate systm
//  geometry::CoordinateSystem const& cs{env.GetCoordinateSystem()};
//
//  // a test particle
//  const auto pcode{particles::Code::Electron};
//  const auto mass{particles::Electron::GetMass()};
//
//  // create a new stack for each trial
//  setup::Stack stack;
//
//  // construct an energy
//  const HEPEnergyType E0{1_TeV};
//
//  // compute the necessary momentumn
//  const HEPMomentumType P0{sqrt(E0 * E0 - mass * mass)};
//
//  // and create the momentum vector
//  const auto plab{corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, P0})};
//
//  // and create the location of the particle in this coordinate system
//  const geometry::Point pos(cs, 5_m, 1_m, 8_m);
//
//  // add the particle to the stack
//  auto particle{stack.AddParticle(std::make_tuple(pcode, E0, plab, pos, 0_ns))};
//
//  // get a view into the stack
//  corsika::stack::SecondaryView stackview(particle);
//
//  // create a trajectory
//  const auto& track{
//      Trajectory(Line(pos, VelocityVec(cs, 0_m / 1_s, 0_m / 1_s, 0.1_m / 1_ns)), 1_ns)};
//
//  // and create a view vector
//  const auto& view{Vector(cs, 0_m, 0_m, 1_m)};
//
//  // construct a radio detector instance to store our antennas
//  TimeDomainDetector<TimeDomainAntenna> detector;
//
//  // the antenna location
//  const auto point{geometry::Point(cs, 1_m, 2_m, 3_m)};
//
//  // check that I can create an antenna at (1, 2, 3)
//  const auto ant{TimeDomainAntenna("antenna_name", point)};
//
//  // add this antenna to the process
//  detector.AddAntenna(ant);
//
//  // create a TimeDomain process
//  auto zhs{TimeDomain<ZHS<CPU>, decltype(detector)>(detector)};
//
//  // get the projectile
//  auto projectile{stackview.GetProjectile()};
//
//  // call ZHS over the track
//  zhs.DoContinuous(projectile, track);
//  zhs.Simulate(projectile, track);
//
//  // try and call the ZHS implementation directly for a given view direction
//  ZHS<CPU>::Emit(projectile, track, view);

//////////////////////////////////////////////////////////////////////////////////////////


  // create an environment with uniform refractive index of 1
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
