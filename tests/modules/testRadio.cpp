/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#include <catch2/catch.hpp>

#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/AntennaCollection.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SimplePropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

#include <vector>
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
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IRefractiveIndexModel.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ExponentialRefractiveIndex.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>

#include <corsika/output/OutputManager.hpp>

using namespace corsika;

double constexpr absMargin = 1.0e-7;

template <typename TInterface>
using MyExtraEnv =
    UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;

TEST_CASE("Radio", "[processes]") {

  logging::set_level(logging::level::debug);

  SECTION("CoREAS process") {

    // This serves as a compiler test for any changes in the CoREAS algorithm
    // Environment
    using EnvironmentInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;

    //    using EnvType = setup::Environment;
    using EnvType = Environment<EnvironmentInterface>;
    EnvType envCoREAS;
    CoordinateSystemPtr const& rootCSCoREAS = envCoREAS.getCoordinateSystem();
    Point const center{rootCSCoREAS, 0_m, 0_m, 0_m};

    //        1.000327,
    create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
        envCoREAS, AtmosphereId::LinsleyUSStd, center, 1.000327, Medium::AirDry1Atm,
        MagneticFieldVector{rootCSCoREAS, 0_T, 50_uT, 0_T});

    // now create antennas and detectors
    // the antennas location
    const auto point1{Point(envCoREAS.getCoordinateSystem(), 100_m, 2_m, 3_m)};
    const auto point2{Point(envCoREAS.getCoordinateSystem(), 4_m, 80_m, 6_m)};
    const auto point3{Point(envCoREAS.getCoordinateSystem(), 7_m, 8_m, 9_m)};
    const auto point4{Point(envCoREAS.getCoordinateSystem(), 5_m, 5_m, 10_m)};

    // create times for the antenna
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    const TimeType t4{11_s};

    // check that I can create an antenna at (1, 2, 3)
    TimeDomainAntenna ant1("antenna_name", point1, rootCSCoREAS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name2", point2, rootCSCoREAS, t1, t2, t3, t1);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    // create a particle
    const Code particle{Code::Electron};
    // const Code particle{Code::Proton};
    const auto pmass{get_mass(particle)};

    VelocityVector v0(rootCSCoREAS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCSCoREAS, 5_T, 5_T, 5_T);

    Line const line(point3, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 1e-12_s;
    LeapFrogTrajectory base(point4, v0, B0, k, t);
    // std::cout << "Leap Frog Trajectory is: " << base << std::endl;

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};

    // and create the momentum vector
    const auto plab{MomentumVector(rootCSCoREAS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCSCoREAS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(
        particle, calculate_kinetic_energy(plab.getNorm(), get_mass(particle)),
        plab.normalized(), pos, 0_ns))};

    auto const charge_{get_charge(particle1.getPID())};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector),
                 CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))>,
                 decltype(StraightPropagator(envCoREAS))>
        coreas(detector, envCoREAS);

    Step step(particle1, base);
    // check doContinuous and simulate methods
    coreas.doContinuous(step, true);
  } // END: SECTION("CoREAS process")

  SECTION("ZHS process") {

    // This section serves as a compiler test for any changes in the ZHS algorithm
    // Environment
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
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
    NuclearComposition const protonComposition({Code::Proton}, {1.});

    // create magnetic field vector
    Vector B1(rootCSZHS, 0_T, 0_T, 1_T);

    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());

    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1,
                                                            density, protonComposition);
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
    TimeDomainAntenna ant1("antenna_zhs", point1, rootCSZHS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_zhs2", point2, rootCSZHS, t1, t2, t3, t1);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    // create a particle
    auto const particle{Code::Electron};
    const auto pmass{get_mass(particle)};

    VelocityVector v0(rootCSZHS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCSZHS, 5_T, 5_T, 5_T);

    Line const line(point3, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 1e-12_s;
    LeapFrogTrajectory base(point4, v0, B0, k, t);
    // std::cout << "Leap Frog Trajectory is: " << base << std::endl;

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentum
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};

    // and create the momentum vector
    const auto plab{MomentumVector(rootCSZHS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCSZHS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(
        particle, calculate_kinetic_energy(plab.getNorm(), get_mass(particle)),
        plab.normalized(), pos, 0_ns))};

    auto const charge_{get_charge(particle1.getPID())};

    // create a radio process instance using ZHS
    RadioProcess<
        AntennaCollection<TimeDomainAntenna>,
        ZHS<AntennaCollection<TimeDomainAntenna>, decltype(StraightPropagator(envZHS))>,
        decltype(StraightPropagator(envZHS))>
        zhs(detector, envZHS);

    Step step(particle1, base);
    // check doContinuous and simulate methods
    zhs.doContinuous(step, true);

  } // END: SECTION("ZHS process")

    SECTION("Radio extreme cases") {

        // Environment
        using EnvironmentInterface =
                IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;

        using EnvType = Environment<EnvironmentInterface>;
        EnvType envRadio;
        CoordinateSystemPtr const& rootCSRadio = envRadio.getCoordinateSystem();
        Point const center{rootCSRadio, 0_m, 0_m, 0_m};

        create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
                envRadio, AtmosphereId::LinsleyUSStd, center, 1.415, Medium::AirDry1Atm,
                MagneticFieldVector{rootCSRadio, 0_T, 50_uT, 0_T});

        // now create antennas and detectors
        // the antennas location
        const auto point1{Point(envRadio.getCoordinateSystem(), 0_m, 0_m, 0_m)};

        // track points
        Point const point_1(rootCSRadio, {1_m, 1_m, 0_m});
        Point const point_2(rootCSRadio, {2_km, 1_km, 0_m});
        Point const point_4(rootCSRadio, {0_m, 1_m, 0_m});

        // create times for the antenna
        const TimeType start{0_s};
        const TimeType duration{100_ns};
        const InverseTimeType sample{1e+12_Hz};
        const TimeType duration_dummy{2_s};
        const InverseTimeType sample_dummy{1_Hz};

        // check that I can create an antenna at (1, 2, 3)
        TimeDomainAntenna ant1("antenna_name", point1, rootCSRadio, start, duration, sample, start);
        TimeDomainAntenna ant2("dummy", point1, rootCSRadio, start, duration_dummy, sample_dummy, start);
        // construct a radio detector instance to store our antennas
        AntennaCollection<TimeDomainAntenna> detector;
        AntennaCollection<TimeDomainAntenna> detector_dummy;
        // add the antennas to the detector
        detector.addAntenna(ant1);
        detector_dummy.addAntenna(ant2);

        // create a new stack for each trial
        setup::Stack<EnvType> stack;
        // create a particle
        const Code particle{Code::Electron};
        const Code particle2{Code::Proton};

        const auto pmass{get_mass(particle)};
        const auto pmass2{get_mass(particle2)};
        // construct an energy
        const HEPEnergyType E0{1_TeV};
        // compute the necessary momentumn
        const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
        // and create the momentum vector
        const auto plab{MomentumVector(rootCSRadio, {P0, 0_GeV, 0_GeV})};
        // add the particle to the stack
        auto const particle_stack{stack.addParticle(std::make_tuple(
                particle, calculate_kinetic_energy(plab.getNorm(), get_mass(particle)),
                plab.normalized(), point_1, 0_ns))};

        // particle stack with proton
        auto const particle_stack_proton{stack.addParticle(std::make_tuple(
                particle2, calculate_kinetic_energy(plab.getNorm(), get_mass(particle2)),
                plab.normalized(), point_1, 0_ns))};

        // feed radio with a zero length trajectory to trigger the startTime = endTime check.
        TimeType t0{(point_1 - point_1).getNorm() / (0.999 * constants::c)};
        VelocityVector v{(point_1 - point_1) / t0};
        Line l{point_1, v};
        StraightTrajectory track0{l, t0};
        Step step0(particle_stack, track0);

        // feed radio with a proton track to check that it skips that track.
        TimeType tp{(point_2 - point_1).getNorm() / (0.999 * constants::c)};
        VelocityVector vp{(point_2 - point_1) / tp};
        Line lp{point_1, vp};
        StraightTrajectory track_p{lp, tp};
        Step step_proton(particle_stack_proton, track_p);

        // feed radio with a track that triggers zhs like approx in coreas and fraunhofer limit check for zhs
        TimeType th{(point_4 - point1).getNorm() / constants::c};
        VelocityVector vh{(point_4 - point1) / th};
        Line lh{point1, vh};
        StraightTrajectory track_h{lh, th};
        Step step_h(particle_stack, track_h);

        // create radio process instances
        RadioProcess<decltype(detector),
                CoREAS<decltype(detector), decltype(SimplePropagator(envRadio))>,
                decltype(SimplePropagator(envRadio))>
                coreas(detector, envRadio);

        RadioProcess<decltype(detector),
                ZHS<decltype(detector), decltype(SimplePropagator(envRadio))>,
                decltype(SimplePropagator(envRadio))>
                zhs(detector, envRadio);

        coreas.doContinuous(step0, true);
        zhs.doContinuous(step0, true);
        coreas.doContinuous(step_proton, true);
        zhs.doContinuous(step_proton, true);
        coreas.doContinuous(step_h, true);
        zhs.doContinuous(step_h, true);

        // create radio processes with "dummy" antenna to trigger extreme time-binning
        RadioProcess<decltype(detector_dummy),
                CoREAS<decltype(detector_dummy), decltype(SimplePropagator(envRadio))>,
                decltype(SimplePropagator(envRadio))>
                coreas_dummy(detector_dummy, envRadio);

        RadioProcess<decltype(detector_dummy),
                ZHS<decltype(detector_dummy), decltype(SimplePropagator(envRadio))>,
                decltype(SimplePropagator(envRadio))>
                zhs_dummy(detector_dummy, envRadio);

        coreas_dummy.doContinuous(step_h, true);
        zhs_dummy.doContinuous(step_h, true);

    } // END: SECTION("Radio extreme cases")

} // END: TEST_CASE("Radio", "[processes]")

TEST_CASE("Antennas") {

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
                1, 1_kg / (1_m * 1_m * 1_m), NuclearComposition({Code::Nitrogen}, {1.}));

        env6.getUniverse()->addChild(std::move(Medium6));

        // create times for the antenna
        const TimeType t1{10_s};
        const TimeType t2{10_s};
        const InverseTimeType t3{1 / 1_s};
        const TimeType t4{11_s};

        // check that I can create an antenna at (1, 2, 3)
        TimeDomainAntenna ant1("antenna_name", point1, rootCS6, t1, t2, t3, t1);
        TimeDomainAntenna ant2("antenna_name2", point2, rootCS6, t4, t2, t3, t4);

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
        Vector<ElectricFieldType::dimension_type> v11(rootCS6,
                                                      {10_V / 1_m, 10_V / 1_m, 10_V / 1_m});

        Vector<dimensionless_d> v2(rootCS6, {0, 1, 0});
        Vector<ElectricFieldType::dimension_type> v22(rootCS6,
                                                      {20_V / 1_m, 20_V / 1_m, 20_V / 1_m});

        // use receive methods
        ant1.receive(15_s, v1, v11);
        ant2.receive(16_s, v2, v22);

        // use getDataX,Y,Z() and getAxis() methods
        auto Ex = ant1.getWaveformX();
        CHECK(Ex[5] - 10 == 0);
        auto tx = ant1.getAxis();
        CHECK(tx[5] - 5 * 1_s / 1_ns == Approx(0.0));
        auto Ey = ant1.getWaveformY();
        CHECK(Ey[5] - 10 == 0);
        auto Ez = ant1.getWaveformZ();
        CHECK(Ez[5] - 10 == 0);
        auto ty = ant1.getAxis();
        auto tz = ant1.getAxis();
        CHECK(tx[5] - ty[5] == 0);
        CHECK(ty[5] - tz[5] == 0);
        auto Ex2 = ant2.getWaveformX();
        CHECK(Ex2[5] - 20 == 0);
        auto Ey2 = ant2.getWaveformY();
        CHECK(Ey2[5] - 20 == 0);
        auto Ez2 = ant2.getWaveformZ();
        CHECK(Ez2[5] - 20 == 0);

        // the following creates a star-shaped pattern of antennas in the ground
        AntennaCollection<TimeDomainAntenna> detector__;
        const auto point11{Point(env6.getCoordinateSystem(), 1000_m, 20_m, 30_m)};
        const TimeType t2222{1e-6_s};
        const InverseTimeType t3333{1e+9_Hz};

        std::vector<std::string> antenna_names;
        std::vector<Point> antenna_locations;
        for (auto radius_ = 100_m; radius_ <= 200_m; radius_ += 100_m) {
            for (auto phi_ = 0; phi_ <= 315; phi_ += 45) {
                auto phiRad_ = phi_ / 180. * M_PI;
                auto const point_{Point(env6.getCoordinateSystem(), radius_ * cos(phiRad_),
                                        radius_ * sin(phiRad_), 0_m)};
                antenna_locations.push_back(point_);
                auto time__{(point11 - point_).getNorm() / constants::c};
                const int rr_ = static_cast<int>(radius_ / 1_m);
                std::string var_ = "antenna_R=" + std::to_string(rr_) +
                                   "_m-Phi=" + std::to_string(phi_) + "degrees";
                antenna_names.push_back(var_);
                TimeDomainAntenna ant111(var_, point_, rootCS6, time__, t2222, t3333, time__);
                detector__.addAntenna(ant111);
            }
        }

        CHECK(detector__.size() == 16);
        CHECK(detector__.getAntennas().size() == 16);
        int i = 0;
        // this prints out the antenna names and locations
        for (auto const antenna: detector__.getAntennas()) {
            CHECK(antenna.getName() == antenna_names[i]);
            CHECK(distance(antenna.getLocation(), antenna_locations[i]) / 1_m == 0);
            i++;
        }

        // Check the .at() method for radio detectors
        for (size_t i = 0; i <= (detector__.size()-1); i++) {
          CHECK(detector__.at(i).getName() == antenna_names[i]);
          CHECK(distance(detector__.at(i).getLocation(), antenna_locations[i]) / 1_m == 0);
        }

    } // END: SECTION("TimeDomainAntenna")

} // END: TEST_CASE("Antennas")

TEST_CASE("Propagators") {

  SECTION("Simple Propagator w/ Uniform Refractive Index") {

    // create a suitable environment
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
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
    NuclearComposition const Composition({Code::Nitrogen}, {1.});
    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 0.3809_T);
    // create a Sphere for the medium
    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());
    // set the environment properties
    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1,
                                                            density, Composition);
    // bind things together
    env.getUniverse()->addChild(std::move(Medium));

    // get some points
    Point p0(rootCS, {0_m, 0_m, 0_m});
    Point p10(rootCS, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> v1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> v2(rootCS, {0, 0, -1});

    // get a geometrical path of points
    Path P1({p0, p10});

    // construct a Straight Propagator given the uniform refractive index environment
    SimplePropagator SP(env);

    // store the outcome of the Propagate method to paths_
    auto const paths_ = SP.propagate(p0, p10, 1_m);

    // perform checks to paths_ components
    for (auto const& path : paths_) {
      CHECK((path.propagation_time_ / 1_s) -
                (((p10 - p0).getNorm() / constants::c) / 1_s) ==
            Approx(0));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.emit_.getComponents() == v1.getComponents());
      CHECK(path.receive_.getComponents() == v2.getComponents());
      CHECK(path.R_distance_ == 10_m);
      CHECK(std::equal(P1.begin(), P1.end(), Path(path.points_).begin(),
                       [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    }

  } // END: SECTION("Simple Propagator w/ Uniform Refractive Index")

  // check that I can create working Straight Propagators in different environments
  SECTION("Straight Propagator w/ Uniform Refractive Index") {

    // create a suitable environment
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
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
    NuclearComposition const Composition({Code::Nitrogen}, {1.});
    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 0.3809_T);
    // create a Sphere for the medium
    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());
    // set the environment properties
    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1,
                                                            density, Composition);
    // bind things together
    env.getUniverse()->addChild(std::move(Medium));

    // get some points
    Point p0(rootCS, {0_m, 0_m, 0_m});
    Point p1(rootCS, {0_m, 0_m, 1_m});
    Point p2(rootCS, {0_m, 0_m, 2_m});
    Point p3(rootCS, {0_m, 0_m, 3_m});
    Point p4(rootCS, {0_m, 0_m, 4_m});
    Point p5(rootCS, {0_m, 0_m, 5_m});
    Point p6(rootCS, {0_m, 0_m, 6_m});
    Point p7(rootCS, {0_m, 0_m, 7_m});
    Point p8(rootCS, {0_m, 0_m, 8_m});
    Point p9(rootCS, {0_m, 0_m, 9_m});
    Point p10(rootCS, {0_m, 0_m, 10_m});
    Point p30(rootCS, {0_m, 0_m, 30000_m});

    // get a unit vector
    Vector<dimensionless_d> v1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> v2(rootCS, {0, 0, -1});

    // get a geometrical path of points
    Path P1({p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10});

    // construct a Straight Propagator given the uniform refractive index environment
    StraightPropagator SP(env);

    // store the outcome of the Propagate method to paths_
    auto const paths_ = SP.propagate(p0, p10, 1_m);

    // perform checks to paths_ components
    for (auto const& path : paths_) {
      CHECK((path.propagation_time_ / 1_s) -
                (((p10 - p0).getNorm() / constants::c) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.emit_.getComponents() == v1.getComponents());
      CHECK(path.receive_.getComponents() == v2.getComponents());
      CHECK(path.R_distance_ == 10_m);
      CHECK(std::equal(P1.begin(), P1.end(), Path(path.points_).begin(),
                       [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    }

    // get another path to different points
    auto const paths2_{SP.propagate(p0, p30, 909_m)};

    for (auto const& path : paths2_) {
      CHECK((path.propagation_time_ / 1_s) -
                (((p30 - p0).getNorm() / constants::c) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.R_distance_ == 30000_m);
    }

    // get a third path using a weird stepsize
    auto const paths3_{SP.propagate(p0, p30, 731.89_m)};

    for (auto const& path : paths3_) {
      CHECK((path.propagation_time_ / 1_s) -
                (((p30 - p0).getNorm() / constants::c) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.R_distance_ == 30000_m);
    }

    CHECK(paths_.size() == 1);
    CHECK(paths2_.size() == 1);
    CHECK(paths3_.size() == 1);
  } // END: SECTION("Straight Propagator w/ Uniform Refractive Index")

  SECTION("Straight Propagator w/ Exponential Refractive Index") {

    // create an environment with exponential refractive index (n_0 = 1 & lambda = 0)
    using ExpoRIndex = ExponentialRefractiveIndex<
        HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env1;

    // get another coordinate system
    const CoordinateSystemPtr rootCS1 = env1.getCoordinateSystem();

    // the center of the earth
    Point const center1_{rootCS1, 0_m, 0_m, 0_m};
    LengthType const radius_{0_m};

    auto Medium1 = EnvType::createNode<Sphere>(
        Point{rootCS1, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props1 = Medium1->setModelProperties<ExpoRIndex>(
        1, 0 / 1_m, center1_, radius_, 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition({Code::Nitrogen}, {1.}));

    env1.getUniverse()->addChild(std::move(Medium1));

    // get some points
    Point pp0(rootCS1, {0_m, 0_m, 0_m});
    Point pp1(rootCS1, {0_m, 0_m, 1_m});
    Point pp2(rootCS1, {0_m, 0_m, 2_m});
    Point pp3(rootCS1, {0_m, 0_m, 3_m});
    Point pp4(rootCS1, {0_m, 0_m, 4_m});
    Point pp5(rootCS1, {0_m, 0_m, 5_m});
    Point pp6(rootCS1, {0_m, 0_m, 6_m});
    Point pp7(rootCS1, {0_m, 0_m, 7_m});
    Point pp8(rootCS1, {0_m, 0_m, 8_m});
    Point pp9(rootCS1, {0_m, 0_m, 9_m});
    Point pp10(rootCS1, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> vv1(rootCS1, {0, 0, 1});
    Vector<dimensionless_d> vv2(rootCS1, {0, 0, -1});

    // get a geometrical path of points
    Path PP1({pp0, pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10});

    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator SP1(env1);

    // store the outcome of Propagate method to paths1_
    auto const paths1_ = SP1.propagate(pp0, pp10, 1_m);

    // perform checks to paths1_ components (this is just a sketch for now)
    for (auto const& path : paths1_) {
      CHECK((path.propagation_time_ / 1_s) -
                (((pp10 - pp0).getNorm() / constants::c) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(1));
      CHECK(path.refractive_index_source_ == Approx(1));
      CHECK(path.refractive_index_destination_ == Approx(1));
      CHECK(path.emit_.getComponents() == vv1.getComponents());
      CHECK(path.receive_.getComponents() == vv2.getComponents());
      CHECK(path.R_distance_ == 10_m);
      CHECK(std::equal(PP1.begin(), PP1.end(), Path(path.points_).begin(),
                       [](Point a, Point b) { return (a - b).getNorm() / 1_m < 1e-5; }));
    }

    CHECK(paths1_.size() == 1);

    /*
     * A second environment with another exponential refractive index
     */

    // create an environment with exponential refractive index (n_0 = 2 & lambda = 2)
    using ExpoRIndex = ExponentialRefractiveIndex<
        HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env2;

    // get another coordinate system
    const CoordinateSystemPtr rootCS2 = env2.getCoordinateSystem();

    // the center of the earth
    Point const center2_{rootCS2, 0_m, 0_m, 0_m};

    auto Medium2 = EnvType::createNode<Sphere>(
        Point{rootCS2, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props2 = Medium2->setModelProperties<ExpoRIndex>(
        2, 2 / 1_m, center2_, radius_, 1_kg / (1_m * 1_m * 1_m),
        NuclearComposition({Code::Nitrogen}, {1.}));

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
    for (auto const& path : paths2_) {
      CHECK((path.propagation_time_ / 1_s) -
                ((3.177511688_m / (3 * constants::c)) / 1_s) ==
            Approx(0).margin(absMargin));
      CHECK(path.average_refractive_index_ == Approx(0.210275935));
      CHECK(path.refractive_index_source_ == Approx(2));
      // CHECK(path.refractive_index_destination_ == Approx(0.0000000041));
      CHECK(path.emit_.getComponents() == vvv1.getComponents());
      CHECK(path.receive_.getComponents() == vvv2.getComponents());
      CHECK(path.R_distance_ == 10_m);
    }

    CHECK(paths2_.size() == 1);

  } // END: SECTION("Straight Propagator w/ Exponential Refractive Index")

} // END: TEST_CASE("Propagators")
