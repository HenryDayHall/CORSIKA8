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

#include <boost/filesystem.hpp>
#include <vector>
#include <istream>
#include <filesystem>
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

    using EnvType = Environment<EnvironmentInterface>;
    //    using EnvType = setup::Environment;
    EnvType envCoREAS;
    CoordinateSystemPtr const& rootCS = envCoREAS.getCoordinateSystem();
    Point const center{rootCS, 0_m, 0_m, 0_m};

    create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
        envCoREAS, AtmosphereId::LinsleyUSStd, center, 1.000327, Medium::AirDry1Atm,
        MagneticFieldVector{rootCS, 0_T, 50_uT, 0_T});

    // create the detector
    const auto ant1Loc{Point(rootCS, 100_m, 2_m, 3_m)};
    const auto ant2Loc{Point(rootCS, 4_m, 80_m, 6_m)};
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    TimeDomainAntenna ant1("antenna_name", ant1Loc, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name2", ant2Loc, rootCS, t1, t2, t3, t1);
    AntennaCollection<TimeDomainAntenna> detector;
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    const auto trackStart{Point(rootCS, 7_m, 8_m, 9_m)};
    const auto trackEnd{Point(rootCS, 5_m, 5_m, 10_m)};

    // create an electron
    const Code electron{Code::Electron};
    const auto pmass{get_mass(electron)};

    VelocityVector v0(rootCS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCS, 5_T, 5_T, 5_T);

    Line const line(trackStart, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 1e-12_s;
    LeapFrogTrajectory base(trackEnd, v0, B0, k, t);

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
    const auto plab{MomentumVector(rootCS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(
        electron, calculate_kinetic_energy(plab.getNorm(), get_mass(electron)),
        plab.normalized(), pos, 0_ns))};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector),
                 CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))>,
                 decltype(StraightPropagator(envCoREAS))>
        coreas(detector, envCoREAS);

    Step step(particle1, base);
    // check doContinuous and simulate methods
    auto const result = coreas.doContinuous(step, true);
    REQUIRE(ProcessReturn::Ok == result);

    for (auto const& ant : detector.getAntennas()) {
      // make sure something was put into the antenna
      auto totalX = ant.getWaveformX()[0];
      auto totalY = ant.getWaveformY()[0];
      auto totalZ = ant.getWaveformZ()[0];
      for (size_t i = 0; i < ant.getWaveformX().size(); i++) {
        totalX += ant.getWaveformX()[i];
        totalY += ant.getWaveformY()[i];
        totalZ += ant.getWaveformZ()[i];
      }
      REQUIRE((totalX + totalY + totalZ) > (totalX * 0));
    }

    //////////////////////////////////////
    // reset everything for a new particle
    //////////////////////////////////////
    ant1.reset();
    ant2.reset();
    stack.purge();

    // add the particle to the stack that is VERY late
    auto const particle2{stack.addParticle(std::make_tuple(
        electron, calculate_kinetic_energy(plab.getNorm(), get_mass(electron)),
        plab.normalized(), pos, t1 + t2 * 100000))};
    Step step2(particle2, base);
    auto const result2 = coreas.doContinuous(step2, true);
    REQUIRE(ProcessReturn::Ok == result2);
    for (auto const& ant : detector.getAntennas()) {
      // make sure something was put into the antenna
      auto total = ant.getWaveformX()[0];
      for (size_t i = 0; i < ant.getWaveformX().size(); i++) {
        total += ant.getWaveformX()[i] * ant.getWaveformX()[i];
        total += ant.getWaveformY()[i] * ant.getWaveformY()[i];
        total += ant.getWaveformZ()[i] * ant.getWaveformZ()[i];
      }
      REQUIRE(total < (1e-12 * ant.getWaveformX().size()));
    }

    coreas.endOfLibrary();

  } // END: SECTION("CoREAS process")

  SECTION("CoREAS Edge Cases") {
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType envCoREAS;
    CoordinateSystemPtr const& rootCS = envCoREAS.getCoordinateSystem();
    Point const center{rootCS, 0_m, 0_m, 0_m};

    Vector B1(rootCS, 0_T, 0_T, 1_T);
    NuclearComposition const protonComposition({Code::Proton}, {1.});
    const double refractiveIndex{1.000327};
    const auto density{1_g / cube(1_cm)};
    auto Medium = EnvType::createNode<Sphere>(
        center, 10_km * std::numeric_limits<double>::infinity());
    auto const props = Medium->setModelProperties<AtmModel>(
        refractiveIndex, Medium::AirDry1Atm, B1, density, protonComposition);
    envCoREAS.getUniverse()->addChild(std::move(Medium));

    // create the detector
    const auto ant1Loc{Point(rootCS, 100_m, 2_m, 3_m)};
    const auto ant2Loc{Point(rootCS, 4_m, 80_m, 6_m)};
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    TimeDomainAntenna ant1("antenna_name", ant1Loc, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name2", ant2Loc, rootCS, t1, t2, t3, t1);
    AntennaCollection<TimeDomainAntenna> detector;
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    const auto trackStart{Point(rootCS, 7_m, 8_m, 9_m)};
    const auto trackEnd{Point(rootCS, 5_m, 5_m, 10_m)};

    // create an electron
    const Code electron{Code::Electron};
    const auto pmass{get_mass(electron)};

    VelocityVector v0(rootCS, {1_m / second, 0_m / second, 0_m / second});

    Vector B0(rootCS, 5_T, 5_T, 5_T);

    Line const line(trackStart, v0);

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
    const auto plab{MomentumVector(rootCS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(
        electron, calculate_kinetic_energy(plab.getNorm(), get_mass(electron)),
        plab.normalized(), pos, 0_ns))};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector),
                 CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))>,
                 decltype(StraightPropagator(envCoREAS))>
        coreas(detector, envCoREAS);

    auto result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, 0_ns, 0_ns, v0, v0)), true);
    REQUIRE(ProcessReturn::Ok == result);
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, 0_ns, 1_ns, v0, v0)), true);
    REQUIRE(ProcessReturn::Ok == result);
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, 0_ns, -1_ns, v0, v0)), true);
    REQUIRE(ProcessReturn::Ok == result);
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, 1_ns, -1_ns, v0, v0)), true);
    REQUIRE(ProcessReturn::Ok == result);
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, -1_ns, 1_ns, v0, v0)), true);
    REQUIRE(ProcessReturn::Ok == result);
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(line, -1_ns, 1_ns, v0, -v0)), true);
    REQUIRE(ProcessReturn::Ok == result);

    // Use ZHS-like loop
    auto const vParallel =
        VelocityVector(rootCS, {0_m / second, 1_m / second, 0_m / second});
    result = coreas.doContinuous(
        Step(particle1, StraightTrajectory(Line(trackStart, vParallel), 0_ns, 1_ns,
                                           vParallel, vParallel)),
        true);
    REQUIRE(ProcessReturn::Ok == result);
  }

  SECTION("ZHS process") {

    // This section serves as a compiler test for any changes in the ZHS algorithm
    // Environment
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType envZHS;
    CoordinateSystemPtr const& rootCS = envZHS.getCoordinateSystem();
    // get the center point
    Point const center{rootCS, 0_m, 0_m, 0_m};
    // a refractive index
    const double ri_{1.000327};

    // the constant density
    const auto density{19.2_g / cube(1_cm)};

    // the composition we use for the homogeneous medium
    NuclearComposition const protonComposition({Code::Proton}, {1.});

    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 1_T);

    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());

    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1,
                                                            density, protonComposition);
    envZHS.getUniverse()->addChild(std::move(Medium));

    // the antennas location
    const auto trackStart{Point(envZHS.getCoordinateSystem(), 7_m, 8_m, 9_m)};
    const auto trackEnd{Point(envZHS.getCoordinateSystem(), 5_m, 5_m, 10_m)};

    // create the detector
    const auto ant1Loc{Point(rootCS, 100_m, 2_m, 3_m)};
    const auto ant2Loc{Point(rootCS, 4_m, 80_m, 6_m)};
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    TimeDomainAntenna ant1("antenna_name", ant1Loc, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name2", ant2Loc, rootCS, t1, t2, t3, t1);
    AntennaCollection<TimeDomainAntenna> detector;
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    // create a particle
    auto const particle{Code::Electron};
    const auto pmass{get_mass(particle)};

    VelocityVector v0(rootCS, {5e+2_m / second, 5e+2_m / second, 5e+2_m / second});

    Vector B0(rootCS, 5_T, 5_T, 5_T);

    Line const line(trackStart, v0);

    auto const k{1_m * ((1_m) / ((1_s * 1_s) * 1_V))};

    auto const t = 1e-12_s;
    LeapFrogTrajectory base(trackEnd, v0, B0, k, t);

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentum
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
    const auto plab{MomentumVector(rootCS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCS, 50_m, 10_m, 80_m);

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
    TimeDomainAntenna ant1("antenna_name", point1, rootCSRadio, start, duration, sample,
                           start);
    TimeDomainAntenna ant2("dummy", point1, rootCSRadio, start, duration_dummy,
                           sample_dummy, start);
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

    // feed radio with a proton track to check that it skips that track.
    TimeType tp{(point_2 - point_1).getNorm() / (0.999 * constants::c)};
    VelocityVector vp{(point_2 - point_1) / tp};
    Line lp{point_1, vp};
    StraightTrajectory track_p{lp, tp};
    Step step_proton(particle_stack_proton, track_p);

    // feed radio with a track that triggers zhs like approx in coreas and fraunhofer
    // limit check for zhs
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

  SECTION("Process Library") {
    using IModelInterface =
        IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<
        MediumPropertyModel<UniformMagneticField<HomogeneousMedium<IModelInterface>>>>;
    using EnvType = Environment<AtmModel>;
    EnvType envCoREAS;
    CoordinateSystemPtr const& rootCS = envCoREAS.getCoordinateSystem();
    Point const center{rootCS, 0_m, 0_m, 0_m};

    Vector B1(rootCS, 0_T, 0_T, 1_T);
    NuclearComposition const protonComposition({Code::Proton}, {1.});
    const double refractiveIndex{1.000327};
    const auto density{1_g / cube(1_cm)};
    auto Medium = EnvType::createNode<Sphere>(
        center, 10_km * std::numeric_limits<double>::infinity());
    auto const props = Medium->setModelProperties<AtmModel>(
        refractiveIndex, Medium::AirDry1Atm, B1, density, protonComposition);
    envCoREAS.getUniverse()->addChild(std::move(Medium));

    // create the detector
    const auto ant1Loc{Point(rootCS, 100_m, 2_m, 3_m)};
    const auto ant2Loc{Point(rootCS, 4_m, 80_m, 6_m)};
    const TimeType t1{0_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1e+3_Hz};
    TimeDomainAntenna ant1("antenna_name", ant1Loc, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name2", ant2Loc, rootCS, t1, t2, t3, t1);
    AntennaCollection<TimeDomainAntenna> detector;
    detector.addAntenna(ant1);
    detector.addAntenna(ant2);

    const auto trackStart{Point(rootCS, 7_m, 8_m, 9_m)};
    const auto trackEnd{Point(rootCS, 5_m, 5_m, 10_m)};

    // create an electron
    const Code electron{Code::Electron};
    const auto pmass{get_mass(electron)};

    VelocityVector v0(rootCS, {1_m / second, 0_m / second, 0_m / second});

    Vector B0(rootCS, 5_T, 5_T, 5_T);

    Line const line(trackStart, v0);

    // create a new stack for each trial
    setup::Stack<EnvType> stack;

    // construct an energy
    const HEPEnergyType E0{1_TeV};

    // compute the necessary momentumn
    const HEPMomentumType P0{sqrt(E0 * E0 - pmass * pmass)};
    const auto plab{MomentumVector(rootCS, {0_GeV, 0_GeV, P0})};

    // and create the location of the particle in this coordinate system
    const Point pos(rootCS, 50_m, 10_m, 80_m);

    // add the particle to the stack
    auto const particle1{stack.addParticle(std::make_tuple(
        electron, calculate_kinetic_energy(plab.getNorm(), get_mass(electron)),
        plab.normalized(), pos, 0_ns))};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector),
                 CoREAS<decltype(detector), decltype(StraightPropagator(envCoREAS))>,
                 decltype(StraightPropagator(envCoREAS))>
        coreas(detector, envCoREAS);

    const auto config = coreas.getConfig();

    CHECK(config["type"].as<std::string>() == "RadioProcess");
    CHECK(config["algorithm"].as<std::string>() == "CoREAS");
    CHECK(config["units"]["time"].as<std::string>() == "ns");
    CHECK(config["units"]["frequency"].as<std::string>() == "GHz");
    CHECK(config["units"]["electric field"].as<std::string>() == "V/m");
    CHECK(config["units"]["distance"].as<std::string>() == "m");

    CHECK(config["antennas"]["antenna_name"]["location"][0].as<double>() == 100);
    CHECK(config["antennas"]["antenna_name"]["location"][1].as<double>() == 2);
    CHECK(config["antennas"]["antenna_name"]["location"][2].as<double>() == 3);

    CHECK(config["antennas"]["antenna_name2"]["location"][0].as<double>() == 4);
    CHECK(config["antennas"]["antenna_name2"]["location"][1].as<double>() == 80);
    CHECK(config["antennas"]["antenna_name2"]["location"][2].as<double>() == 6);
  }

} // END: TEST_CASE("Radio", "[processes]")

TEST_CASE("Antennas") {

  SECTION("TimeDomainAntenna Constructor") {
    Environment<IRefractiveIndexModel<IMediumModel>> env;
    const auto rootCS = env.getCoordinateSystem();
    auto const antPos = Point(rootCS, {0_m, 0_m, 0_m});
    TimeType const tStart(0_s);
    TimeType const duration(10_ns);
    InverseTimeType const sampleRate(1_GHz);
    TimeType const groundHitTime(1e3_ns);

    TimeDomainAntenna const antenna("antenna", antPos, rootCS, tStart, duration,
                                    sampleRate, groundHitTime);

    // All waveforms are of equal non-zero size
    CHECK(antenna.getWaveformX().size() == antenna.getWaveformY().size());
    CHECK(antenna.getWaveformX().size() == antenna.getWaveformZ().size());
    CHECK(antenna.getWaveformX().size() > 0);

    // All waveform values are initialized to zero
    for (auto const& val : antenna.getWaveformX()) { CHECK(val * 0 == val); }
    for (auto const& val : antenna.getWaveformY()) { CHECK(val * 0 == val); }
    for (auto const& val : antenna.getWaveformZ()) { CHECK(val * 0 == val); }

    // check that variables were set properly
    CHECK("antenna" == antenna.getName());
    CHECK(sampleRate == antenna.getSampleRate());
    CHECK(tStart == antenna.getStartTime());

    // and check that the antenna is at the right location
    CHECK((antenna.getLocation() - antPos).getNorm() < 1e-12 * 1_m);
  } // END: SECTION("TimeDomainAntenna Constructor")

  SECTION("TimeDomainAntenna Bad Constructor") {
    Environment<IRefractiveIndexModel<IMediumModel>> env;
    const auto rootCS = env.getCoordinateSystem();
    auto const antPos = Point(rootCS, {0_m, 0_m, 0_m});
    TimeType const tStart(0_s);
    TimeType const duration(1e3_ns);
    InverseTimeType const sampleRate(1_GHz);
    TimeType const groundHitTime(10_ns);

    // Giving zero or negative values for sampling rate and duration
    TimeDomainAntenna const antenna_bad1("bad_antenna", antPos, rootCS, tStart, -13_ns,
                                         sampleRate, groundHitTime);
    TimeDomainAntenna const antenna_bad2("bad_antenna", antPos, rootCS, tStart, 0_ns,
                                         sampleRate, groundHitTime);
    TimeDomainAntenna const antenna_bad3("bad_antenna", antPos, rootCS, tStart, duration,
                                         -1_GHz, groundHitTime);
  } // END: SECTION("TimeDomainAntenna Bad Constructor")

  SECTION("TimeDomainAntenna Receive Efield") {
    // Checks that the basic functionality of the receive function is working properly

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env;

    const auto rootCS = env.getCoordinateSystem();

    auto const point1 = Point(rootCS, {1_m, 2_m, 3_m});
    auto const point2 = Point(rootCS, {4_m, 5_m, 6_m});

    // create times for the antenna
    const TimeType t1{10_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1 / 1_s};
    const TimeType t4{11_s};

    // make the two antennas with different start times
    TimeDomainAntenna ant1("antenna_name", point1, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name", point2, rootCS, t4, t2, t3, t4);

    Vector<dimensionless_d> receiveVec1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> receiveVec2(rootCS, {0, 1, 0});

    Vector<ElectricFieldType::dimension_type> eField1(
        rootCS, {10_V / 1_m, 10_V / 1_m, 10_V / 1_m});
    Vector<ElectricFieldType::dimension_type> eField2(
        rootCS, {20_V / 1_m, 20_V / 1_m, 20_V / 1_m});

    // inject efield into ant1
    ant1.receive(15_s, receiveVec1, eField1);
    REQUIRE(ant1.getWaveformX()[5] - 10 == 0);
    REQUIRE(ant1.getWaveformX()[5] == ant1.getWaveformY()[5]);
    REQUIRE(ant1.getWaveformX()[5] == ant1.getWaveformZ()[5]);

    // inject efield but with different receive vector into ant2
    ant2.receive(16_s, receiveVec2, eField1);
    REQUIRE(ant1.getWaveformX()[5] ==
            ant2.getWaveformX()[5]); // Currently receive vector does nothing
    ant2.reset();
    REQUIRE(ant2.getWaveformX()[5] == 0); // reset was successful

    // inject the other eField into ant2
    ant2.receive(16_s, receiveVec2, eField2);
    REQUIRE(ant2.getWaveformX()[5] - 20 == 0);
    REQUIRE(ant2.getWaveformX()[5] == ant2.getWaveformY()[5]);
    REQUIRE(ant2.getWaveformX()[5] == ant2.getWaveformZ()[5]);

    // make sure the next one is empty before filling it
    REQUIRE(ant2.getWaveformX()[6] == 0);
    ant2.receive(17_s, receiveVec2, eField2);
    REQUIRE(ant2.getWaveformX()[6] - 20 == 0);

    // reset ant1 and then put values in out of range
    ant1.reset();
    ant1.receive(-1000_s, receiveVec1, eField1);
    for (auto const& val : ant1.getWaveformX()) { CHECK(val * 0 == val); }
    ant1.reset();
    ant1.receive(t1 + t2 + 1_s, receiveVec1, eField1);
    for (auto const& val : ant1.getWaveformX()) { CHECK(val * 0 == val); }
  } // END: SECTION("TimeDomainAntenna Receive EField")

  SECTION("TimeDomainAntenna Receive Vector Potential") {
    // Checks that the basic functionality of the receive function is working properly

    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env;

    const auto rootCS = env.getCoordinateSystem();

    auto const point1 = Point(rootCS, {1_m, 2_m, 3_m});
    auto const point2 = Point(rootCS, {4_m, 5_m, 6_m});

    // create times for the antenna
    const TimeType t1{10_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1 / 1_s};
    const TimeType t4{11_s};

    // make the two antennas with different start times
    TimeDomainAntenna ant1("antenna_name", point1, rootCS, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna_name", point2, rootCS, t4, t2, t3, t4);

    Vector<dimensionless_d> receiveVec1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> receiveVec2(rootCS, {0, 1, 0});

    Vector<VectorPotentialType::dimension_type> vectorPotential1(
        rootCS, {10_V * 1_s / 1_m, 10_V * 1_s / 1_m, 10_V * 1_s / 1_m});
    Vector<VectorPotentialType::dimension_type> vectorPotential2(
        rootCS, {20_V * 1_s / 1_m, 20_V * 1_s / 1_m, 20_V * 1_s / 1_m});

    // inject efield into ant1
    ant1.receive(15_s, receiveVec1, vectorPotential1);
    REQUIRE(ant1.getWaveformX()[5] - 10 == 0);
    REQUIRE(ant1.getWaveformX()[5] == ant1.getWaveformY()[5]);
    REQUIRE(ant1.getWaveformX()[5] == ant1.getWaveformZ()[5]);

    // inject efield but with different receive vector into ant2
    ant2.receive(16_s, receiveVec2, vectorPotential1);
    REQUIRE(ant1.getWaveformX()[5] ==
            ant2.getWaveformX()[5]); // Currently receive vector does nothing
    ant2.reset();
    REQUIRE(ant2.getWaveformX()[5] == 0); // reset was successful

    // inject the other eField into ant2
    ant2.receive(16_s, receiveVec2, vectorPotential2);
    REQUIRE(ant2.getWaveformX()[5] - 20 == 0);
    REQUIRE(ant2.getWaveformX()[5] == ant2.getWaveformY()[5]);
    REQUIRE(ant2.getWaveformX()[5] == ant2.getWaveformZ()[5]);

    // make sure the next one is empty before filling it
    REQUIRE(ant2.getWaveformX()[6] == 0);
    ant2.receive(17_s, receiveVec2, vectorPotential2);
    REQUIRE(ant2.getWaveformX()[6] - 20 == 0);

    // reset ant1 and then put values in out of range
    ant1.reset();
    ant1.receive(-1000_s, receiveVec1, vectorPotential1);
    for (auto const& val : ant1.getWaveformX()) { CHECK(val * 0 == val); }
    ant1.reset();
    ant1.receive(t1 + t2 + 1_s, receiveVec1, vectorPotential1);
    for (auto const& val : ant1.getWaveformX()) { CHECK(val * 0 == val); }
  } // END: SECTION("TimeDomainAntenna Receive Vector Potential")

  SECTION("TimeDomainAntenna AntennaCollection") {

    // create an environment so we can get a coordinate system
    using EnvType = Environment<IRefractiveIndexModel<IMediumModel>>;
    EnvType env6;

    using UniRIndex =
        UniformRefractiveIndex<HomogeneousMedium<IRefractiveIndexModel<IMediumModel>>>;

    // the antenna location
    const auto point1{Point(env6.getCoordinateSystem(), 1_m, 2_m, 3_m)};
    const auto point2{Point(env6.getCoordinateSystem(), 4_m, 5_m, 6_m)};

    // get a coordinate system
    const CoordinateSystemPtr rootCS = env6.getCoordinateSystem();

    auto Medium6 = EnvType::createNode<Sphere>(
        Point{rootCS, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

    auto const props6 = Medium6->setModelProperties<UniRIndex>(
        1, 1_kg / (1_m * 1_m * 1_m), NuclearComposition({Code::Nitrogen}, {1.}));

    env6.getUniverse()->addChild(std::move(Medium6));

    // create times for the antenna
    const TimeType t1{10_s};
    const TimeType t2{10_s};
    const InverseTimeType t3{1 / 1_s};
    const TimeType t4{11_s};

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // the following creates a star-shaped pattern of antennas in the ground
    const auto point11{Point(env6.getCoordinateSystem(), 1000_m, 20_m, 30_m)};
    const TimeType t2222{1e-6_s};
    const InverseTimeType t3333{1e+9_Hz};

    std::vector<std::string> antenna_names;
    std::vector<Point> antenna_locations;
    for (auto radius = 100_m; radius <= 200_m; radius += 100_m) {
      for (auto phi = 0; phi <= 315; phi += 45) {
        auto phiRad = phi / 180. * M_PI;
        auto const point{Point(env6.getCoordinateSystem(), radius * cos(phiRad),
                               radius * sin(phiRad), 0_m)};
        antenna_locations.push_back(point);
        auto time__{(point11 - point).getNorm() / constants::c};
        const int rr_ = static_cast<int>(radius / 1_m);
        std::string name = "antenna_R=" + std::to_string(rr_) +
                           "_m-Phi=" + std::to_string(phi) + "degrees";
        antenna_names.push_back(name);
        TimeDomainAntenna ant(name, point, rootCS, time__, t2222, t3333, time__);
        detector.addAntenna(ant);
      }
    }

    CHECK(detector.size() == 16);
    CHECK(detector.getAntennas().size() == 16);
    int i = 0;
    // this prints out the antenna names and locations
    for (auto const& antenna : detector.getAntennas()) {
      CHECK(antenna.getName() == antenna_names[i]);
      CHECK(distance(antenna.getLocation(), antenna_locations[i]) / 1_m == 0);
      i++;
    }

    // Check the .at() method for radio detectors
    for (int i = 0; i <= (detector.size() - 1); i++) {
      CHECK(detector.at(i).getName() == antenna_names[i]);
      CHECK(distance(detector.at(i).getLocation(), antenna_locations[i]) / 1_m == 0);
    }

  } // END: SECTION("TimeDomainAntenna AntennaCollection")

  SECTION("TimeDomainAntenna Library") {
    // Runs checks that the file readers are working properly
    Environment<IRefractiveIndexModel<IMediumModel>> env;
    const auto rootCS = env.getCoordinateSystem();
    auto const antPos = Point(rootCS, {0_m, 0_m, 0_m});
    TimeType const tStart(0_s);
    TimeType const duration(10_ns);
    InverseTimeType const sampleRate(1_GHz);
    TimeType const groundHitTime(1e3_ns);

    TimeDomainAntenna antenna("test_antenna", antPos, rootCS, tStart, duration,
                              sampleRate, groundHitTime);

    // Run the start of lib and end of shower functions on the antenna
    std::vector<std::string> implementations{"CoREAS", "ZHS"};
    for (auto const implemen : implementations) {
      // For each implementation type, save a file
      boost::filesystem::path const tempPath{boost::filesystem::temp_directory_path() /
                                             ("test_corsika_radio_" + implemen)};
      if (boost::filesystem::exists(tempPath)) {
        boost::filesystem::remove_all(tempPath);
      }
      boost::filesystem::create_directory(tempPath);
      antenna.startOfLibrary(tempPath, implemen);
      auto const outputFile = tempPath / (antenna.getName() + ".npz");
      CHECK(boost::filesystem::exists(outputFile));

      // run end of shower and make sure that something extra was added
      auto const fileSize = boost::filesystem::file_size(outputFile);
      antenna.endOfShower(0, implemen, sampleRate / 1_Hz);
      CHECK(boost::filesystem::file_size(outputFile) > fileSize);
    }

    // Check the YAML file output
    auto const config = antenna.getConfig();
    CHECK(config["type"].as<std::string>() == "TimeDomainAntenna");
    CHECK(config["start_time"].as<double>() == tStart / 1_ns);
    CHECK(config["duration"].as<double>() == duration / 1_ns);
    CHECK(config["sample_rate"].as<double>() == sampleRate / 1_GHz);
  } // END: SECTION("TimeDomainAntenna Library")

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
    Point const p0(rootCS, {0_m, 0_m, 0_m});
    Point const p10(rootCS, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> const v1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> const v2(rootCS, {0, 0, -1});

    // get a geometrical path of points
    Path const P1({p0, p10});

    // construct a Straight Propagator given the uniform refractive index environment
    SimplePropagator const SP(env);

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
      CHECK(std::equal(
          P1.begin(), P1.end(), path.begin(),
          [](Point const& a, Point const& b) { return (a - b).getNorm() / 1_m < 1e-5; }));
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
    Point const p0(rootCS, {0_m, 0_m, 0_m});
    Point const p1(rootCS, {0_m, 0_m, 1_m});
    Point const p2(rootCS, {0_m, 0_m, 2_m});
    Point const p3(rootCS, {0_m, 0_m, 3_m});
    Point const p4(rootCS, {0_m, 0_m, 4_m});
    Point const p5(rootCS, {0_m, 0_m, 5_m});
    Point const p6(rootCS, {0_m, 0_m, 6_m});
    Point const p7(rootCS, {0_m, 0_m, 7_m});
    Point const p8(rootCS, {0_m, 0_m, 8_m});
    Point const p9(rootCS, {0_m, 0_m, 9_m});
    Point const p10(rootCS, {0_m, 0_m, 10_m});
    Point const p30(rootCS, {0_m, 0_m, 30000_m});

    // get a unit vector
    Vector<dimensionless_d> const v1(rootCS, {0, 0, 1});
    Vector<dimensionless_d> const v2(rootCS, {0, 0, -1});

    // get a geometrical path of points
    Path const P1({p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10});

    // construct a Straight Propagator given the uniform refractive index environment
    StraightPropagator const SP(env);

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
      CHECK(std::equal(
          P1.begin(), P1.end(), path.begin(),
          [](Point const& a, Point const& b) { return (a - b).getNorm() / 1_m < 1e-5; }));
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
    Point const pp0(rootCS1, {0_m, 0_m, 0_m});
    Point const pp1(rootCS1, {0_m, 0_m, 1_m});
    Point const pp2(rootCS1, {0_m, 0_m, 2_m});
    Point const pp3(rootCS1, {0_m, 0_m, 3_m});
    Point const pp4(rootCS1, {0_m, 0_m, 4_m});
    Point const pp5(rootCS1, {0_m, 0_m, 5_m});
    Point const pp6(rootCS1, {0_m, 0_m, 6_m});
    Point const pp7(rootCS1, {0_m, 0_m, 7_m});
    Point const pp8(rootCS1, {0_m, 0_m, 8_m});
    Point const pp9(rootCS1, {0_m, 0_m, 9_m});
    Point const pp10(rootCS1, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> vv1(rootCS1, {0, 0, 1});
    Vector<dimensionless_d> vv2(rootCS1, {0, 0, -1});

    // get a geometrical path of points
    Path const PP1({pp0, pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10});

    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator const SP1(env1);

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
      CHECK(std::equal(
          PP1.begin(), PP1.end(), path.begin(),
          [](Point const& a, Point const& b) { return (a - b).getNorm() / 1_m < 1e-5; }));
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
    Point const ppp0(rootCS2, {0_m, 0_m, 0_m});
    Point const ppp10(rootCS2, {0_m, 0_m, 10_m});

    // get a unit vector
    Vector<dimensionless_d> const vvv1(rootCS2, {0, 0, 1});
    Vector<dimensionless_d> const vvv2(rootCS2, {0, 0, -1});

    // construct a Straight Propagator given the exponential refractive index environment
    StraightPropagator const SP2(env2);

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
