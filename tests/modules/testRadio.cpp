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
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    const TimeType t4{11_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_name", point1, t1, t2, t3);
    TimeDomainAntenna ant2("antenna_name2", point2, t1, t2, t3);
//    TimeDomainAntenna ant3("antenna1", point1, 0_s, 2_s, 1/1e-7_s);

//    std::cout << "static cast " << static_cast<int>(1/1000) << std::endl;

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

    auto const t = 1_s;
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
    coreas.doContinuous(particle1, base, true);
//    coreas1.simulate(particle1, base);

    // check writeOutput method -> should produce 2 csv files for each antenna
    coreas.writeOutput();
    }


  SECTION("ZHS process") {

    //////////////////////////////////////////////////////////////////////////////////////
    // Environment
    using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
        <IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType envZHS;
    CoordinateSystemPtr const& rootCSZHS = envZHS.getCoordinateSystem();
    // get the center point
    Point const center{rootCSZHS, 0_m, 0_m, 0_m};
    // a refractive index
    const double ri_{1.000327};

    // the constant density
    const auto density{19.2_g / cube(1_cm)};

    // the composition we use for the homogeneous medium
    NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                               std::vector<float>{1.f});

    // create magnetic field vector
    Vector B1(rootCSZHS, 0_T, 0_T, 1_T);

    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());

    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1, density, protonComposition);
    envZHS.getUniverse()->addChild(std::move(Medium));

    // the antennas location
    const auto point1{Point(envZHS.getCoordinateSystem(), 100_m, 2_m, 3_m)};
    const auto point2{Point(envZHS.getCoordinateSystem(), 4_m, 80_m, 6_m)};
    const auto point3{Point(envZHS.getCoordinateSystem(), 7_m, 8_m, 9_m)};
    const auto point4{Point(envZHS.getCoordinateSystem(), 5_m, 5_m, 10_m)};

    // create times for the antenna
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    const TimeType t4{11_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_zhs", point1, t1, t2, t3);
    TimeDomainAntenna ant2("antenna_zhs2", point2, t1, t2, t3);
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

    VelocityVector v0(rootCSZHS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCSZHS, 5_T, 5_T, 5_T);

    Line const line(point3, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 1_s;
    LeapFrogTrajectory base(point4, v0, B0, k, t);

    // create a new stack for each trial
    setup::Stack stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};

    // and create the momentum vector
    const auto plab{MomentumVector(rootCSZHS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCSZHS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(particle, E0, plab, pos, 0_ns))};

    auto const charge_ {get_charge(particle1.getPID())};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector), ZHS<decltype(detector), decltype(StraightPropagator(envZHS))>, decltype(StraightPropagator(envZHS))>
        zhs(detector, envZHS);

    // check doContinuous and simulate methods
    zhs.doContinuous(particle1, base, true);
//    zhs.simulate(particle1, base);

    // check writeOutput method -> should produce 2 csv files for each antenna
    zhs.writeOutput();
  }


  SECTION("Synchrotron radiation") {

    // create a suitable environment ///////////////////////////////////////////////////
    using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
        <IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType env;
    CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
    // get the center point
    Point const center{rootCS, 0_m, 0_m, 0_m};
    // a refractive index for the vacuum
    const double ri_{1};
    // the constant density
    const auto density{19.2_g / cube(1_cm)};
    // the composition we use for the homogeneous medium
    NuclearComposition const Composition(std::vector<Code>{Code::Nitrogen},
                                         std::vector<float>{1.f});
    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 0.3809_T);
    // create a Sphere for the medium
    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());
    // set the environment properties
    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1, density, Composition);
    // bind things together
    env.getUniverse()->addChild(std::move(Medium));


    // now create antennas and detectors/////////////////////////////////////////////
    // the antennas location
    const auto point1{Point(rootCS, 100_m, 100_m, 0_m)};
    const auto point2{Point(rootCS, 100_m, -100_m, 0_m)};
    const auto point3{Point(rootCS, -100_m, -100_m, 0_m)};
    const auto point4{Point(rootCS, -100_m, 100_m, 0_m)};


    // create times for the antenna
    const TimeType t1{0_s};
    const TimeType t2{1e-6_s};
    const InverseTimeType t3{1e+9_Hz};

    // create 4 cool antennas
    TimeDomainAntenna ant1("cool antenna", point1, t1, t2, t3);
    TimeDomainAntenna ant2("cooler antenna", point2, t1, t2, t3);
    TimeDomainAntenna ant3("coolest antenna", point3, t1, t2, t3);
    TimeDomainAntenna ant4("No, I am the coolest antenna", point4, t1, t2, t3);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);
    detector.addAntenna(ant3);
    detector.addAntenna(ant4);



    // create points that make a 2D circle of radius=100m ////////////////////////////////
    Point p0(rootCS, {0_m, 100_m, 0_m});
    Point p1(rootCS, {1_m, 99.995_m, 0_m});
    Point p2(rootCS, {2_m,99.98_m, 0_m});
    Point p3(rootCS, {3_m,99.955_m, 0_m});
    Point p4(rootCS, {4_m,99.92_m, 0_m});
    Point p5(rootCS, {5_m,99.875_m, 0_m});
    Point p6(rootCS, {6_m,99.82_m, 0_m});
    Point p7(rootCS, {7_m,99.755_m, 0_m});
    Point p8(rootCS, {8_m,99.679_m, 0_m});
    Point p9(rootCS, {9_m,99.594_m, 0_m});
    Point p10(rootCS,{10_m,99.499_m, 0_m});
    Point p11(rootCS,{11_m,99.393_m, 0_m});
    Point p12(rootCS,{12_m,99.277_m, 0_m});
    Point p13(rootCS,{13_m,99.151_m, 0_m});
    Point p14(rootCS,{14_m,99.015_m, 0_m});
    Point p15(rootCS,{15_m,98.869_m, 0_m});
    Point p16(rootCS,{16_m,98.712_m, 0_m});
    Point p17(rootCS,{17_m,98.544_m, 0_m});
    Point p18(rootCS,{18_m,98.367_m, 0_m});
    Point p19(rootCS,{19_m,98.178_m, 0_m});
    Point p20(rootCS,{20_m,97.98_m, 0_m});
    Point p21(rootCS,{21_m,97.77_m, 0_m});
    Point p22(rootCS,{22_m,97.55_m, 0_m});
    Point p23(rootCS,{23_m,97.319_m, 0_m});
    Point p24(rootCS,{24_m,97.077_m, 0_m});
    Point p25(rootCS,{25_m,96.825_m, 0_m});
    Point p26(rootCS,{26_m,96.561_m, 0_m});
    Point p27(rootCS,{27_m,96.286_m, 0_m});
    Point p28(rootCS,{28_m,96_m, 0_m});
    Point p29(rootCS,{29_m,95.703_m, 0_m});
    Point p30(rootCS,{30_m,95.394_m, 0_m});
    Point p31(rootCS,{31_m,95.074_m, 0_m});
    Point p32(rootCS,{32_m,94.742_m, 0_m});
    Point p33(rootCS,{33_m,94.398_m, 0_m});
    Point p34(rootCS,{34_m,94.043_m, 0_m});
    Point p35(rootCS,{35_m,93.675_m, 0_m});
    Point p36(rootCS,{36_m,93.295_m, 0_m});
    Point p37(rootCS,{37_m,92.903_m, 0_m});
    Point p38(rootCS,{38_m,92.499_m, 0_m});
    Point p39(rootCS,{39_m,92.081_m, 0_m});
    Point p40(rootCS,{40_m,91.652_m, 0_m});
    Point p41(rootCS,{41_m,91.209_m, 0_m});
    Point p42(rootCS,{42_m,90.752_m, 0_m});
    Point p43(rootCS,{43_m,90.283_m, 0_m});
    Point p44(rootCS,{44_m,89.8_m, 0_m});
    Point p45(rootCS,{45_m,89.303_m, 0_m});
    Point p46(rootCS,{46_m,88.792_m, 0_m});
    Point p47(rootCS,{47_m,88.267_m, 0_m});
    Point p48(rootCS,{48_m,87.727_m, 0_m});
    Point p49(rootCS,{49_m,87.171_m, 0_m});
    Point p50(rootCS,{50_m,86.603_m, 0_m});
    Point p51(rootCS,{51_m,86.017_m, 0_m});
    Point p52(rootCS,{52_m,85.417_m, 0_m});
    Point p53(rootCS,{53_m,84.8_m, 0_m});
    Point p54(rootCS,{54_m,84.167_m, 0_m});
    Point p55(rootCS,{55_m,83.516_m, 0_m});
    Point p56(rootCS,{56_m,82.849_m, 0_m});
    Point p57(rootCS,{57_m,82.164_m, 0_m});
    Point p58(rootCS,{58_m,81.462_m, 0_m});
    Point p59(rootCS,{59_m,80.74_m, 0_m});
    Point p60(rootCS,{60_m,80_m, 0_m});
    Point p61(rootCS,{61_m,79.24_m, 0_m});
    Point p62(rootCS,{62_m,78.46_m, 0_m});
    Point p63(rootCS,{63_m,77.66_m, 0_m});
    Point p64(rootCS,{64_m,76.837_m, 0_m});
    Point p65(rootCS,{65_m,75.993_m, 0_m});
    Point p66(rootCS,{66_m,75.127_m, 0_m});
    Point p67(rootCS,{67_m,74.236_m, 0_m});
    Point p68(rootCS,{68_m,73.321_m, 0_m});
    Point p69(rootCS,{69_m,72.476_m, 0_m});
    Point p70(rootCS,{70_m,71.414_m, 0_m});
    Point p71(rootCS,{71_m,70.42_m, 0_m});
    Point p72(rootCS,{72_m,69.397_m, 0_m});
    Point p73(rootCS,{73_m,68.345_m, 0_m});
    Point p74(rootCS,{74_m,67.261_m, 0_m});
    Point p75(rootCS,{75_m,66.144_m, 0_m});
    Point p76(rootCS,{76_m,64.992_m, 0_m});
    Point p77(rootCS,{77_m,63.804_m, 0_m});
    Point p78(rootCS,{78_m,62.578_m, 0_m});
    Point p79(rootCS,{79_m,61.311_m, 0_m});
    Point p80(rootCS,{80_m,60_m, 0_m});
    Point p81(rootCS,{81_m,58.643_m, 0_m});
    Point p82(rootCS,{82_m,57.236_m, 0_m});
    Point p83(rootCS,{83_m,55.776_m, 0_m});
    Point p84(rootCS,{84_m,54.259_m, 0_m});
    Point p85(rootCS,{85_m,52.678_m, 0_m});
    Point p86(rootCS,{86_m,51.029_m, 0_m});
    Point p87(rootCS,{87_m,49.305_m, 0_m});
    Point p88(rootCS,{88_m,47.497_m, 0_m});
    Point p89(rootCS,{89_m,45.596_m, 0_m});
    Point p90(rootCS,{90_m,43.589_m, 0_m});
    Point p91(rootCS,{91_m,41.461_m, 0_m});
    Point p92(rootCS,{92_m,39.192_m, 0_m});
    Point p93(rootCS,{93_m,36.756_m, 0_m});
    Point p94(rootCS,{94_m,34.117_m, 0_m});
    Point p95(rootCS,{95_m,31.225_m, 0_m});
    Point p96(rootCS,{96_m,28_m, 0_m});
    Point p97(rootCS,{97_m,24.31_m, 0_m});
    Point p98(rootCS,{98_m,19.9_m, 0_m});
    Point p99(rootCS,{99_m,14.107_m, 0_m});
    Point p100(rootCS,{100_m,0_m, 0_m});
    Point p101(rootCS,{99_m,-14.107_m, 0_m});
    Point p102(rootCS,{98_m,-19.9_m, 0_m});
    Point p103(rootCS,{97_m,-24.31_m, 0_m});
    Point p104(rootCS,{96_m,-28_m, 0_m});
    Point p105(rootCS,{95_m,-31.225_m, 0_m});
    Point p106(rootCS,{94_m,-34.117_m, 0_m});
    Point p107(rootCS,{93_m,-36.756_m, 0_m});
    Point p108(rootCS,{92_m,-39.192_m, 0_m});
    Point p109(rootCS,{91_m,-41.461_m, 0_m});
    Point p110(rootCS,{90_m,-43.589_m, 0_m});
    Point p111(rootCS,{89_m,-45.596_m, 0_m});
    Point p112(rootCS,{88_m,-47.497_m, 0_m});
    Point p113(rootCS,{87_m,-49.305_m, 0_m});
    Point p114(rootCS,{86_m,-51.029_m, 0_m});
    Point p115(rootCS,{85_m,-52.678_m, 0_m});
    Point p116(rootCS,{84_m,-54.259_m, 0_m});
    Point p117(rootCS,{83_m,-55.776_m, 0_m});
    Point p118(rootCS,{82_m,-57.236_m, 0_m});
    Point p119(rootCS,{81_m,-58.643_m, 0_m});
    Point p120(rootCS,{80_m,-60_m, 0_m});
    Point p121(rootCS,{79_m,-61.311_m, 0_m});
    Point p122(rootCS,{78_m,-62.578_m, 0_m});
    Point p123(rootCS,{77_m,-63.804_m, 0_m});
    Point p124(rootCS,{76_m,-64.992_m, 0_m});
    Point p125(rootCS,{75_m,-66.144_m, 0_m});
    Point p126(rootCS,{74_m,-67.261_m, 0_m});
    Point p127(rootCS,{73_m,-68.345_m, 0_m});
    Point p128(rootCS,{72_m,-69.397_m, 0_m});
    Point p129(rootCS,{71_m,-70.42_m, 0_m});
    Point p130(rootCS,{70_m,-71.414_m, 0_m});
    Point p131(rootCS,{69_m,-72.476_m, 0_m});
    Point p132(rootCS,{68_m,-73.321_m, 0_m});
    Point p133(rootCS,{67_m,-74.236_m, 0_m});
    Point p134(rootCS,{66_m,-75.127_m, 0_m});
    Point p135(rootCS,{65_m,-75.993_m, 0_m});
    Point p136(rootCS,{64_m,-76.837_m, 0_m});
    Point p137(rootCS,{63_m,-77.66_m, 0_m});
    Point p138(rootCS,{62_m,-78.46_m, 0_m});
    Point p139(rootCS,{61_m,-79.24_m, 0_m});
    Point p140(rootCS,{60_m,-80_m, 0_m});
    Point p141(rootCS,{59_m,-80.74_m, 0_m});
    Point p142(rootCS,{58_m,-81.462_m, 0_m});
    Point p143(rootCS,{57_m,-82.164_m, 0_m});
    Point p144(rootCS,{56_m,-82.849_m, 0_m});
    Point p145(rootCS,{55_m,-83.516_m, 0_m});
    Point p146(rootCS,{54_m,-84.167_m, 0_m});
    Point p147(rootCS,{53_m,-84.8_m, 0_m});
    Point p148(rootCS,{52_m,-85.417_m, 0_m});
    Point p149(rootCS,{51_m,-86.017_m, 0_m});
    Point p150(rootCS,{50_m,-86.603_m, 0_m});
    Point p151(rootCS,{49_m,-87.171_m, 0_m});
    Point p152(rootCS,{48_m,-87.727_m, 0_m});
    Point p153(rootCS,{47_m,-88.267_m, 0_m});
    Point p154(rootCS,{46_m,-88.792_m, 0_m});
    Point p155(rootCS,{45_m,-89.303_m, 0_m});
    Point p156(rootCS,{44_m,-89.8_m, 0_m});
    Point p157(rootCS,{43_m,-90.283_m, 0_m});
    Point p158(rootCS,{42_m,-90.752_m, 0_m});
    Point p159(rootCS,{41_m,-91.209_m, 0_m});
    Point p160(rootCS,{40_m,-91.652_m, 0_m});
    Point p161(rootCS,{39_m,-92.081_m, 0_m});
    Point p162(rootCS,{38_m,-92.499_m, 0_m});
    Point p163(rootCS,{37_m,-92.903_m, 0_m});
    Point p164(rootCS,{36_m,-93.295_m, 0_m});
    Point p165(rootCS,{35_m,-93.675_m, 0_m});
    Point p166(rootCS,{34_m,-94.043_m, 0_m});
    Point p167(rootCS,{33_m,-94.398_m, 0_m});
    Point p168(rootCS,{32_m,-94.742_m, 0_m});
    Point p169(rootCS,{31_m,-95.074_m, 0_m});
    Point p170(rootCS,{30_m,-95.394_m, 0_m});
    Point p171(rootCS,{29_m,-95.703_m, 0_m});
    Point p172(rootCS,{28_m,-96_m, 0_m});
    Point p173(rootCS,{27_m,-96.286_m, 0_m});
    Point p174(rootCS,{26_m,-96.561_m, 0_m});
    Point p175(rootCS,{25_m,-96.825_m, 0_m});
    Point p176(rootCS,{24_m,-97.077_m, 0_m});
    Point p177(rootCS,{23_m,-97.319_m, 0_m});
    Point p178(rootCS,{22_m,-97.55_m, 0_m});
    Point p179(rootCS,{21_m,-97.77_m, 0_m});
    Point p180(rootCS,{20_m,-97.98_m, 0_m});
    Point p181(rootCS,{19_m,-98.178_m, 0_m});
    Point p182(rootCS,{18_m,-98.367_m, 0_m});
    Point p183(rootCS,{17_m,-98.544_m, 0_m});
    Point p184(rootCS,{16_m,-98.712_m, 0_m});
    Point p185(rootCS,{15_m,-98.869_m, 0_m});
    Point p186(rootCS,{14_m,-99.015_m, 0_m});
    Point p187(rootCS,{13_m,-99.151_m, 0_m});
    Point p188(rootCS,{12_m,-99.277_m, 0_m});
    Point p189(rootCS,{11_m,-99.393_m, 0_m});
    Point p190(rootCS,{10_m,-99.499_m, 0_m});
    Point p191(rootCS,{9_m,-99.594_m, 0_m});
    Point p192(rootCS,{8_m,-99.679_m, 0_m});
    Point p193(rootCS,{7_m,-99.755_m, 0_m});
    Point p194(rootCS,{6_m,-99.82_m, 0_m});
    Point p195(rootCS,{5_m,-99.875_m, 0_m});
    Point p196(rootCS,{4_m,-99.92_m, 0_m});
    Point p197(rootCS,{3_m,-99.955_m, 0_m});
    Point p198(rootCS,{2_m,-99.98_m, 0_m});
    Point p199(rootCS,{1_m,-99.995_m, 0_m});
    Point p200(rootCS,{0_m,-100_m, 0_m});
    Point p201(rootCS,{-1_m,-99.995_m, 0_m});
    Point p202(rootCS,{-2_m,-99.98_m, 0_m});
    Point p203(rootCS,{-3_m,-99.955_m, 0_m});
    Point p204(rootCS,{-4_m,-99.92_m, 0_m});
    Point p205(rootCS,{-5_m,-99.875_m, 0_m});
    Point p206(rootCS,{-6_m,-99.82_m, 0_m});
    Point p207(rootCS,{-7_m,-99.755_m, 0_m});
    Point p208(rootCS,{-8_m,-99.679_m, 0_m});
    Point p209(rootCS,{-9_m,-99.594_m, 0_m});
    Point p210(rootCS,{-10_m,-99.499_m, 0_m});
    Point p211(rootCS,{-11_m,-99.393_m, 0_m});
    Point p212(rootCS,{-12_m,-99.277_m, 0_m});
    Point p213(rootCS,{-13_m,-99.151_m, 0_m});
    Point p214(rootCS,{-14_m,-99.015_m, 0_m});
    Point p215(rootCS,{-15_m,-98.869_m, 0_m});
    Point p216(rootCS,{-16_m,-98.712_m, 0_m});
    Point p217(rootCS,{-17_m,-98.544_m, 0_m});
    Point p218(rootCS,{-18_m,-98.367_m, 0_m});
    Point p219(rootCS,{-19_m,-98.178_m, 0_m});
    Point p220(rootCS,{-20_m,-97.98_m, 0_m});
    Point p221(rootCS,{-21_m,-97.77_m, 0_m});
    Point p222(rootCS,{-22_m,-97.55_m, 0_m});
    Point p223(rootCS,{-23_m,-97.319_m, 0_m});
    Point p224(rootCS,{-24_m,-97.077_m, 0_m});
    Point p225(rootCS,{-25_m,-96.825_m, 0_m});
    Point p226(rootCS,{-26_m,-96.561_m, 0_m});
    Point p227(rootCS,{-27_m,-96.286_m, 0_m});
    Point p228(rootCS,{-28_m,-96_m, 0_m});
    Point p229(rootCS,{-29_m,-95.703_m, 0_m});
    Point p230(rootCS,{-30_m,-95.394_m, 0_m});
    Point p231(rootCS,{-31_m,-95.074_m, 0_m});
    Point p232(rootCS,{-32_m,-94.742_m, 0_m});
    Point p233(rootCS,{-33_m,-94.398_m, 0_m});
    Point p234(rootCS,{-34_m,-94.043_m, 0_m});
    Point p235(rootCS,{-35_m,-93.675_m, 0_m});
    Point p236(rootCS,{-36_m,-93.295_m, 0_m});
    Point p237(rootCS,{-37_m,-92.903_m, 0_m});
    Point p238(rootCS,{-38_m,-92.499_m, 0_m});
    Point p239(rootCS,{-39_m,-92.081_m, 0_m});
    Point p240(rootCS,{-40_m,-91.652_m, 0_m});
    Point p241(rootCS,{-41_m,-91.209_m, 0_m});
    Point p242(rootCS,{-42_m,-90.752_m, 0_m});
    Point p243(rootCS,{-43_m,-90.283_m, 0_m});
    Point p244(rootCS,{-44_m,-89.8_m, 0_m});
    Point p245(rootCS,{-45_m,-89.303_m, 0_m});
    Point p246(rootCS,{-46_m,-88.792_m, 0_m});
    Point p247(rootCS,{-47_m,-88.267_m, 0_m});
    Point p248(rootCS,{-48_m,-87.727_m, 0_m});
    Point p249(rootCS,{-49_m,-87.171_m, 0_m});
    Point p250(rootCS,{-50_m,-86.603_m, 0_m});
    Point p251(rootCS,{-51_m,-86.017_m, 0_m});
    Point p252(rootCS,{-52_m,-85.417_m, 0_m});
    Point p253(rootCS,{-53_m,-84.8_m, 0_m});
    Point p254(rootCS,{-54_m,-84.167_m, 0_m});
    Point p255(rootCS,{-55_m,-83.516_m, 0_m});
    Point p256(rootCS,{-56_m,-82.849_m, 0_m});
    Point p257(rootCS,{-57_m,-82.164_m, 0_m});
    Point p258(rootCS,{-58_m,-81.462_m, 0_m});
    Point p259(rootCS,{-59_m,-80.74_m, 0_m});
    Point p260(rootCS,{-60_m,-80_m, 0_m});
    Point p261(rootCS,{-61_m,-79.24_m, 0_m});
    Point p262(rootCS,{-62_m,-78.46_m, 0_m});
    Point p263(rootCS,{-63_m,-77.66_m, 0_m});
    Point p264(rootCS,{-64_m,-76.837_m, 0_m});
    Point p265(rootCS,{-65_m,-75.993_m, 0_m});
    Point p266(rootCS,{-66_m,-75.127_m, 0_m});
    Point p267(rootCS,{-67_m,-74.236_m, 0_m});
    Point p268(rootCS,{-68_m,-73.321_m, 0_m});
    Point p269(rootCS,{-69_m,-72.476_m, 0_m});
    Point p270(rootCS,{-70_m,-71.414_m, 0_m});
    Point p271(rootCS,{-71_m,-70.42_m, 0_m});
    Point p272(rootCS,{-72_m,-69.397_m, 0_m});
    Point p273(rootCS,{-73_m,-68.345_m, 0_m});
    Point p274(rootCS,{-74_m,-67.261_m, 0_m});
    Point p275(rootCS,{-75_m,-66.144_m, 0_m});
    Point p276(rootCS,{-76_m,-64.992_m, 0_m});
    Point p277(rootCS,{-77_m,-63.804_m, 0_m});
    Point p278(rootCS,{-78_m,-62.578_m, 0_m});
    Point p279(rootCS,{-79_m,-61.311_m, 0_m});
    Point p280(rootCS,{-80_m,-60_m, 0_m});
    Point p281(rootCS,{-81_m,-58.643_m, 0_m});
    Point p282(rootCS,{-82_m,-57.236_m, 0_m});
    Point p283(rootCS,{-83_m,-55.776_m, 0_m});
    Point p284(rootCS,{-84_m,-54.259_m, 0_m});
    Point p285(rootCS,{-85_m,-52.678_m, 0_m});
    Point p286(rootCS,{-86_m,-51.029_m, 0_m});
    Point p287(rootCS,{-87_m,-49.305_m, 0_m});
    Point p288(rootCS,{-88_m,-47.497_m, 0_m});
    Point p289(rootCS,{-89_m,-45.596_m, 0_m});
    Point p290(rootCS,{-90_m,-43.589_m, 0_m});
    Point p291(rootCS,{-91_m,-41.461_m, 0_m});
    Point p292(rootCS,{-92_m,-39.192_m, 0_m});
    Point p293(rootCS,{-93_m,-36.756_m, 0_m});
    Point p294(rootCS,{-94_m,-34.117_m, 0_m});
    Point p295(rootCS,{-95_m,-31.225_m, 0_m});
    Point p296(rootCS,{-96_m,-28_m, 0_m});
    Point p297(rootCS,{-97_m,-24.31_m, 0_m});
    Point p298(rootCS,{-98_m,-19.9_m, 0_m});
    Point p299(rootCS,{-99_m,-14.107_m, 0_m});
    Point p300(rootCS,{-100_m,0_m, 0_m});
    Point p301(rootCS,{-99_m,14.107_m, 0_m});
    Point p302(rootCS,{-98_m,19.9_m, 0_m});
    Point p303(rootCS,{-97_m,24.31_m, 0_m});
    Point p304(rootCS,{-96_m,28_m, 0_m});
    Point p305(rootCS,{-95_m,31.225_m, 0_m});
    Point p306(rootCS,{-94_m,34.117_m, 0_m});
    Point p307(rootCS,{-93_m,36.756_m, 0_m});
    Point p308(rootCS,{-92_m,39.192_m, 0_m});
    Point p309(rootCS,{-91_m,41.461_m, 0_m});
    Point p310(rootCS,{-90_m,43.589_m, 0_m});
    Point p311(rootCS,{-89_m,45.596_m, 0_m});
    Point p312(rootCS,{-88_m,47.497_m, 0_m});
    Point p313(rootCS,{-87_m,49.305_m, 0_m});
    Point p314(rootCS,{-86_m,51.029_m, 0_m});
    Point p315(rootCS,{-85_m,52.678_m, 0_m});
    Point p316(rootCS,{-84_m,54.259_m, 0_m});
    Point p317(rootCS,{-83_m,55.776_m, 0_m});
    Point p318(rootCS,{-82_m,57.236_m, 0_m});
    Point p319(rootCS,{-81_m,58.643_m, 0_m});
    Point p320(rootCS,{-80_m,60_m, 0_m});
    Point p321(rootCS,{-79_m,61.311_m, 0_m});
    Point p322(rootCS,{-78_m,62.578_m, 0_m});
    Point p323(rootCS,{-77_m,63.804_m, 0_m});
    Point p324(rootCS,{-76_m,64.992_m, 0_m});
    Point p325(rootCS,{-75_m,66.144_m, 0_m});
    Point p326(rootCS,{-74_m,67.261_m, 0_m});
    Point p327(rootCS,{-73_m,68.345_m, 0_m});
    Point p328(rootCS,{-72_m,69.397_m, 0_m});
    Point p329(rootCS,{-71_m,70.42_m, 0_m});
    Point p330(rootCS,{-70_m,71.414_m, 0_m});
    Point p331(rootCS,{-69_m,72.476_m, 0_m});
    Point p332(rootCS,{-68_m,73.321_m, 0_m});
    Point p333(rootCS,{-67_m,74.236_m, 0_m});
    Point p334(rootCS,{-66_m,75.127_m, 0_m});
    Point p335(rootCS,{-65_m,75.993_m, 0_m});
    Point p336(rootCS,{-64_m,76.837_m, 0_m});
    Point p337(rootCS,{-63_m,77.66_m, 0_m});
    Point p338(rootCS,{-62_m,78.46_m, 0_m});
    Point p339(rootCS,{-61_m,79.24_m, 0_m});
    Point p340(rootCS,{-60_m,80_m, 0_m});
    Point p341(rootCS,{-59_m,80.74_m, 0_m});
    Point p342(rootCS,{-58_m,81.462_m, 0_m});
    Point p343(rootCS,{-57_m,82.164_m, 0_m});
    Point p344(rootCS,{-56_m,82.849_m, 0_m});
    Point p345(rootCS,{-55_m,83.516_m, 0_m});
    Point p346(rootCS,{-54_m,84.167_m, 0_m});
    Point p347(rootCS,{-53_m,84.8_m, 0_m});
    Point p348(rootCS,{-52_m,85.417_m, 0_m});
    Point p349(rootCS,{-51_m,86.017_m, 0_m});
    Point p350(rootCS,{-50_m,86.603_m, 0_m});
    Point p351(rootCS,{-49_m,87.171_m, 0_m});
    Point p352(rootCS,{-48_m,87.727_m, 0_m});
    Point p353(rootCS,{-47_m,88.267_m, 0_m});
    Point p354(rootCS,{-46_m,88.792_m, 0_m});
    Point p355(rootCS,{-45_m,89.303_m, 0_m});
    Point p356(rootCS,{-44_m,89.8_m, 0_m});
    Point p357(rootCS,{-43_m,90.283_m, 0_m});
    Point p358(rootCS,{-42_m,90.752_m, 0_m});
    Point p359(rootCS,{-41_m,91.209_m, 0_m});
    Point p360(rootCS,{-40_m,91.652_m, 0_m});
    Point p361(rootCS,{-39_m,92.081_m, 0_m});
    Point p362(rootCS,{-38_m,92.499_m, 0_m});
    Point p363(rootCS,{-37_m,92.903_m, 0_m});
    Point p364(rootCS,{-36_m,93.295_m, 0_m});
    Point p365(rootCS,{-35_m,93.675_m, 0_m});
    Point p366(rootCS,{-34_m,94.043_m, 0_m});
    Point p367(rootCS,{-33_m,94.398_m, 0_m});
    Point p368(rootCS,{-32_m,94.742_m, 0_m});
    Point p369(rootCS,{-31_m,95.074_m, 0_m});
    Point p370(rootCS,{-30_m,95.394_m, 0_m});
    Point p371(rootCS,{-29_m,95.703_m, 0_m});
    Point p372(rootCS,{-28_m,96_m, 0_m});
    Point p373(rootCS,{-27_m,96.286_m, 0_m});
    Point p374(rootCS,{-26_m,96.561_m, 0_m});
    Point p375(rootCS,{-25_m,96.825_m, 0_m});
    Point p376(rootCS,{-24_m,97.077_m, 0_m});
    Point p377(rootCS,{-23_m,97.319_m, 0_m});
    Point p378(rootCS,{-22_m,97.55_m, 0_m});
    Point p379(rootCS,{-21_m,97.77_m, 0_m});
    Point p380(rootCS,{-20_m,97.98_m, 0_m});
    Point p381(rootCS,{-19_m,98.178_m, 0_m});
    Point p382(rootCS,{-18_m,98.367_m, 0_m});
    Point p383(rootCS,{-17_m,98.544_m, 0_m});
    Point p384(rootCS,{-16_m,98.712_m, 0_m});
    Point p385(rootCS,{-15_m,98.869_m, 0_m});
    Point p386(rootCS,{-14_m,99.015_m, 0_m});
    Point p387(rootCS,{-13_m,99.151_m, 0_m});
    Point p388(rootCS,{-12_m,99.277_m, 0_m});
    Point p389(rootCS,{-11_m,99.393_m, 0_m});
    Point p390(rootCS,{-10_m,99.499_m, 0_m});
    Point p391(rootCS,{-9_m,99.594_m, 0_m});
    Point p392(rootCS,{-8_m,99.679_m, 0_m});
    Point p393(rootCS,{-7_m,99.755_m, 0_m});
    Point p394(rootCS,{-6_m,99.82_m, 0_m});
    Point p395(rootCS,{-5_m,99.875_m, 0_m});
    Point p396(rootCS,{-4_m,99.92_m, 0_m});
    Point p397(rootCS,{-3_m,99.955_m, 0_m});
    Point p398(rootCS,{-2_m,99.98_m, 0_m});
    Point p399(rootCS,{-1_m,99.995_m, 0_m});
//    Point p400(rootCS,{0_m,100_m, 0_m}); // same as p0

    // store all the points in a std::array
    std::array<Point, 400> points_
        {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,
         p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,
         p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,
         p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,
         p40,p41,p42,p43,p44,p45,p46,p47,p48,p49,
         p50,p51,p52,p53,p54,p55,p56,p57,p58,p59,
         p60,p61,p62,p63,p64,p65,p66,p67,p68,p69,
         p70,p71,p72,p73,p74,p75,p76,p77,p78,p79,
         p80,p81,p82,p83,p84,p85,p86,p87,p88,p89,
         p90,p91,p92,p93,p94,p95,p96,p97,p98,p99,
         p100,p101,p102,p103,p104,p105,p106,p107,p108,p109,
         p110,p111,p112,p113,p114,p115,p116,p117,p118,p119,
         p120,p121,p122,p123,p124,p125,p126,p127,p128,p129,
         p130,p131,p132,p133,p134,p135,p136,p137,p138,p139,
         p140,p141,p142,p143,p144,p145,p146,p147,p148,p149,
         p150,p151,p152,p153,p154,p155,p156,p157,p158,p159,
         p160,p161,p162,p163,p164,p165,p166,p167,p168,p169,
         p170,p171,p172,p173,p174,p175,p176,p177,p178,p179,
         p180,p181,p182,p183,p184,p185,p186,p187,p188,p189,
         p190,p191,p192,p193,p194,p195,p196,p197,p198,p199,
         p200,p201,p202,p203,p204,p205,p206,p207,p208,p209,
         p210,p211,p212,p213,p214,p215,p216,p217,p218,p219,
         p220,p221,p222,p223,p224,p225,p226,p227,p228,p229,
         p230,p231,p232,p233,p234,p235,p236,p237,p238,p239,
         p240,p241,p242,p243,p244,p245,p246,p247,p248,p249,
         p250,p251,p252,p253,p254,p255,p256,p257,p258,p259,
         p260,p261,p262,p263,p264,p265,p266,p267,p268,p269,
         p270,p271,p272,p273,p274,p275,p276,p277,p278,p279,
         p280,p281,p282,p283,p284,p285,p286,p287,p288,p289,
         p290,p291,p292,p293,p294,p295,p296,p297,p298,p299,
         p300,p301,p302,p303,p304,p305,p306,p307,p308,p309,
         p310,p311,p312,p313,p314,p315,p316,p317,p318,p319,
         p320,p321,p322,p323,p324,p325,p326,p327,p328,p329,
         p330,p331,p332,p333,p334,p335,p336,p337,p338,p339,
         p340,p341,p342,p343,p344,p345,p346,p347,p348,p349,
         p350,p351,p352,p353,p354,p355,p356,p357,p358,p359,
         p360,p361,p362,p363,p364,p365,p366,p367,p368,p369,
         p370,p371,p372,p373,p374,p375,p376,p377,p378,p379,
         p380,p381,p382,p383,p384,p385,p386,p387,p388,p389,
         p390,p391,p392,p393,p394,p395,p396,p397,p398,p399};

    std::vector<TimeType> times_;

    //////////////////////////////////////////////////////////////////////////////////

    // create a new stack for each trial
    setup::Stack stack;
    stack.clear();

    const Code particle{Code::Electron};
    const HEPMassType pmass{get_mass(particle)};

    // construct an energy // move in the for loop
    const HEPEnergyType E0{11.4_MeV};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector), CoREAS<decltype(detector), decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
        coreas(detector, env);

    // loop over all the tracks except the last one
    for (size_t i = 1; i <= 399; i++) {
      TimeType t {(points_[i] - points_[i-1]).getNorm() / (0.999 * constants::c)};
      VelocityVector v { (points_[i] - points_[i-1]) / t };
      auto  beta {v / constants::c};
      auto gamma {E0/pmass};
      auto plab {beta * pmass * gamma};
      Line l {points_[i-1],v};
      StraightTrajectory track {l,t};
      auto particle1{stack.addParticle(std::make_tuple(particle, E0, plab, points_[i-1], t))}; //TODO: plab is inconsistent
      coreas.doContinuous(particle1,track,true);
    }

    // get the last track
    TimeType t {(points_[0] - points_[399]).getNorm() / (0.999 * constants::c)};
    VelocityVector v { (points_[0] - points_[399]) / t };
    auto  beta {v / constants::c};
    auto gamma {E0/pmass};
    auto plab {beta * pmass * gamma};
    Line l {points_[399],v};
    StraightTrajectory track {l,t};
    auto particle1{stack.addParticle(std::make_tuple(particle, E0, plab, points_[399], t))};
    coreas.doContinuous(particle1,track,true);

    // get the output
    coreas.writeOutput();

  }

  SECTION("Synchrotron radiation 2") {

    // create a suitable environment ///////////////////////////////////////////////////
    using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
        <IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType env;
    CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
    // get the center point
    Point const center{rootCS, 0_m, 0_m, 0_m};
    // a refractive index for the vacuum
    const double ri_{1};
    // the constant density
    const auto density{19.2_g / cube(1_cm)};
    // the composition we use for the homogeneous medium
    NuclearComposition const Composition(std::vector<Code>{Code::Nitrogen},
                                         std::vector<float>{1.f});
    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 0.3809_T);
    // create a Sphere for the medium
    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());
    // set the environment properties
    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1, density, Composition);
    // bind things together
    env.getUniverse()->addChild(std::move(Medium));


    // now create antennas and detectors/////////////////////////////////////////////
    // the antennas location
    const auto point1{Point(rootCS, 200_m, 0_m, 0_m)};
//    const auto point1{Point(rootCS, 30000_m, 0_m, 0_m)};
//    const auto point2{Point(rootCS, 5000_m, 100_m, 0_m)};
//    const auto point3{Point(rootCS, -100_m, -100_m, 0_m)};
//    const auto point4{Point(rootCS, -100_m, 100_m, 0_m)};

    // create times for the antenna
//    const TimeType t1{0.998e-4_s};
//    const TimeType t2{1.0000e-4_s};
//    const InverseTimeType t3{1e+11_Hz};

    const TimeType t1{0_s};
    const TimeType t2{1e-6_s};
    const InverseTimeType t3{1e+9_Hz};

    // create 4 cool antennas
    TimeDomainAntenna ant1("cool antenna", point1, t1, t2, t3);
//    TimeDomainAntenna ant2("cooler antenna", point2, t1, t2, t3);
//    TimeDomainAntenna ant3("coolest antenna", point3, t1, t2, t3);
//    TimeDomainAntenna ant4("No, I am the coolest antenna", point4, t1, t2, t3);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
//    detector.addAntenna(ant2);
//    detector.addAntenna(ant3);
//    detector.addAntenna(ant4);

    //////////////////////////////////////////////////////////////////////////////////
    // create a new stack for each trial
    setup::Stack stack;
    stack.clear();

    const Code particle{Code::Electron};
    const HEPMassType pmass{get_mass(particle)};

    // construct an energy // move in the for loop
    const HEPEnergyType E0{11.4_MeV};

    // create a radio process instance using CoREAS or ZHS
    RadioProcess<decltype(detector), CoREAS<decltype(detector), decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
        coreas(detector, env);

    // loop over all the tracks except the last one
    int const n_points {1000};
    LengthType const radius {100_m};
    for (size_t i = 0; i <= n_points; i++) {
      Point const point_1(rootCS,{radius*cos(M_PI*2*i/n_points),radius*sin(M_PI*2*i/n_points), 0_m});
      Point const point_2(rootCS,{radius*cos(M_PI*2*(i+1)/n_points),radius*sin(M_PI*2*(i+1)/n_points), 0_m});
      TimeType t {(point_2 - point_1).getNorm() / (0.999 * constants::c)};
      VelocityVector v { (point_2 - point_1) / t };
      auto  beta {v / constants::c};
      auto gamma {E0/pmass};
      auto plab {beta * pmass * gamma};
      Line l {point_1,v};
      StraightTrajectory track {l,t};
      auto particle1{stack.addParticle(std::make_tuple(particle, E0, plab, point_1, t))};
      coreas.doContinuous(particle1,track,true);
      stack.clear();
    }

    // get the output
    coreas.writeOutput();

  }


  SECTION("TimeDomainAntenna") {

    // create an environment so we can get a coordinate system
    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env6;

    using UniRIndex =
    UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;


    // the antenna location
    const auto point1{Point(env6.getCoordinateSystem(), 1_m, 2_m, 3_m)};
    const auto point2{Point(env6.getCoordinateSystem(), 4_m, 5_m, 6_m)};


    // get a coordinate system
    const CoordinateSystemPtr rootCS6 = env6.getCoordinateSystem();

    auto Medium6 = EnvType::createNode<Sphere>(
        Point{rootCS6, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props6 = Medium6->setModelProperties<UniRIndex>(
        1, 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition(
            std::vector<Code>{Code::Nitrogen},
            std::vector<float>{1.f}));

    env6.getUniverse()->addChild(std::move(Medium6));

    // create times for the antenna
    const TimeType t1{10_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1/1_s};
    const TimeType t4{11_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_name", point1, t1, t2, t3);
    TimeDomainAntenna ant2("antenna_name2", point2, t4, t2, t3);

    // assert that the antenna name is correct
    REQUIRE(ant1.getName() == "antenna_name");
    REQUIRE(ant2.getName() == "antenna_name2");

    // and check that the antenna is at the right location
    REQUIRE((ant1.getLocation() - point1).getNorm() < 1e-12 * 1_m);
    REQUIRE((ant2.getLocation() - point2).getNorm() < 1e-12 * 1_m);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add this antenna to the process
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);
    CHECK(detector.size() == 2);

    // get a unit vector
    Vector<dimensionless_d> v1(rootCS6, {0, 0, 1});
    QuantityVector<ElectricFieldType::dimension_type> v11{10_V / 1_m, 10_V / 1_m, 10_V / 1_m};

    Vector<dimensionless_d> v2(rootCS6, {0, 1, 0});
    QuantityVector<ElectricFieldType::dimension_type> v22{20_V / 1_m, 20_V / 1_m, 20_V / 1_m};

    // use receive methods
    ant1.receive(15_s, v1, v11);
    ant2.receive(16_s, v2, v22);

    // use getWaveform() method
    auto [t111, E1] = ant1.getWaveform();
    CHECK(E1(5,0) - 10 == 0);
    auto [t222, E2] = ant2.getWaveform();
    CHECK(E2(5,0) -20 == 0);

    // use the receive method in a for loop. It works now!
    for (auto& xx : detector.getAntennas()) {
      xx.receive(15_s, v1, v11);
    }

    // t & E are correct! uncomment to see them
//    for (auto& xx : detector.getAntennas()) {
//      auto [t,E] = xx.getWaveform();
//      std::cout << t << std::endl;
//      std::cout << " ... "<< std::endl;
//      std::cout << E << std::endl;
//      std::cout << " ... "<< std::endl;
//    }

    // check output files. It works, uncomment to see.
//    int i = 1;
//    for (auto& antenna : detector.getAntennas()) {
//
//      auto [t,E] = antenna.getWaveform();
//      auto c0 = xt::hstack(xt::xtuple(t,E));
//      std::ofstream out_file ("antenna" + to_string(i) + "_output.csv");
//      xt::dump_csv(out_file, c0);
//      out_file.close();
//      ++i;
//
//    }

    // check reset method for antennas. Uncomment to see they are zero
//    ant1.reset();
//    ant2.reset();
//
//    std::cout << ant1.waveformE_ << std::endl;
//    std::cout << ant2.waveformE_ << std::endl;
//
//    std::cout << " ... "<< std::endl;
//    std::cout << " ... "<< std::endl;

    // check reset method for antenna collection. Uncomment to see they are zero
//    detector.reset();
//    for (auto& xx : detector.getAntennas()) {
//      std::cout << xx.waveformE_ << std::endl;
//      std::cout << " ... "<< std::endl;
//    }


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
    Vector<dimensionless_d> v2(rootCS, {0, 0, -1});

    //    // get a geometrical path of points
    //    Path P1({p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10});

    // construct a Straight Propagator given the uniform refractive index environment
    StraightPropagator SP(env);

    // store the outcome of the Propagate method to paths_
    auto const paths_ = SP.propagate(p0, p10, 1_m);

    // perform checks to paths_ components
    for (auto const& path : paths_) {
      CHECK((path.propagation_time_ / 1_s) - ((34_m / (3 * constants::c)) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.emit_.getComponents() == v1.getComponents());
      CHECK(path.receive_.getComponents() == v2.getComponents());
      CHECK(path.R_distance_ == 10_m);
      //      CHECK(std::equal(P1.begin(), P1.end(), path.points_.begin(),[]
      //      (Point a, Point b) { return (a - b).norm() / 1_m < 1e-5;}));
      //TODO:THINK ABOUT THE POINTS IN THE SIGNALPATH.H

//      std::cout << "path.total_time_: " << path.total_time_ << std::endl;
//      std::cout << "path.average_refractive_index_: " << path.average_refractive_index_ << std::endl;
//      std::cout << "path.emit_: " << path.emit_.getComponents() << std::endl;
//      std::cout << "path.R_distance_: " << path.R_distance_ << std::endl;


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
    Vector<dimensionless_d> vv2(rootCS1, {0, 0, -1});


    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator SP1(env1);

    // store the outcome of Propagate method to paths1_
    auto const paths1_ = SP1.propagate(pp0, pp10, 1_m);

    // perform checks to paths1_ components (this is just a sketch for now)
    for (auto const& path :paths1_) {
      CHECK( (path.propagation_time_ / 1_s)  - ((34_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractive_index_ == Approx(1) );
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK( path.emit_.getComponents() == vv1.getComponents() );
      CHECK( path.receive_.getComponents() == vv2.getComponents() );
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
    Vector<dimensionless_d> vvv2(rootCS2, {0, 0, -1});


    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator SP2(env2);

    // store the outcome of Propagate method to paths1_
    auto const paths2_ = SP2.propagate(ppp0, ppp10, 1_m);

    // perform checks to paths1_ components (this is just a sketch for now)
    for (auto const& path :paths2_) {
      CHECK( (path.propagation_time_ / 1_s)  - ((3.177511688_m / (3 * constants::c)) / 1_s)
             == Approx(0).margin(absMargin) );
      CHECK( path.average_refractive_index_ == Approx(0.210275935) );
      CHECK(path.refractive_index_source_ == Approx(2));
//      CHECK(path.refractive_index_destination_ == Approx(0.0000000041));
      CHECK( path.emit_.getComponents() == vvv1.getComponents() );
      CHECK( path.receive_.getComponents() == vvv2.getComponents() );
      CHECK( path.R_distance_ == 10_m );
    }

    CHECK( paths2_.size() == 1 );

    }

  } // END: TEST_CASE("Radio", "[processes]")
