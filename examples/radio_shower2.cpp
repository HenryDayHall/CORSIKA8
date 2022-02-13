/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/output/OutputManager.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ShowerAxis.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/modules/radio/RadioProcess.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/antennas/Antenna.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/RadioDetector.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SimplePropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/TimeCut.hpp>
//#include <corsika/modules/TrackWriter.hpp>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpppp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>

using namespace corsika;
using namespace std;

//
// A simple shower to get the electric field trace of an electron
//
int main() {

    logging::set_level(logging::level::info);
    corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

    CORSIKA_LOG_INFO("Synchrotron radiation");

    feenableexcept(FE_INVALID);
    RNGManager<>::getInstance().registerRandomStream("cascade");
    std::random_device rd;
    auto seed = rd();
    RNGManager<>::getInstance().setSeed(seed);

    OutputManager output("synchrotron_radiation_output");

    // set up the environment
    using EnvType = setup::Environment;
    EnvType env;
    auto& universe = *(env.getUniverse());
    CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

    auto world = EnvType::createNode<Sphere>(Point{rootCS, 0_m, 0_m, 0_m}, 150_km);

    using MyHomogeneousModel = UniformRefractiveIndex<MediumPropertyModel<
            UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>>;

    auto const Bmag {0.0003809_T};
    MagneticFieldVector B{rootCS, 0_T, 0_T, Bmag};

    // the composition we use for the homogeneous medium
    NuclearComposition const nitrogenComposition({Code::Nitrogen}, {1.});

    world->setModelProperties<MyHomogeneousModel>(1, Medium::AirDry1Atm, B,
                                                  1_kg / (1_m * 1_m * 1_m),
                                                  nitrogenComposition);

    universe.addChild(std::move(world));

    // the antenna locations
    const auto point1{Point(rootCS, 30000_m, 0_m, 0_m)};

    // the antenna time variables
    const TimeType t1{0.994e-4_s};
    const TimeType t2{1.07e-4_s - 0.994e-4_s};
    const InverseTimeType t3{5e+11_Hz};

    // the antennas
    TimeDomainAntenna ant1("antenna CoREAS", point1, t1, t2, t3, t1);
    TimeDomainAntenna ant2("antenna ZHS", point1, t1, t2, t3, t1);

    // the detectors
    AntennaCollection<TimeDomainAntenna> detectorCoREAS;
    AntennaCollection<TimeDomainAntenna> detectorZHS;
    detectorCoREAS.addAntenna(ant1);
    detectorZHS.addAntenna(ant2);


    // setup particle stack, and add primary particle
    setup::Stack stack;
    stack.clear();
    const Code beamCode = Code::Electron;
    auto const charge = get_charge(beamCode);
    auto const mass = get_mass(beamCode);
    auto const gyroradius = 100_m;
    auto const pLabMag = convert_SI_to_HEP(charge * Bmag * gyroradius);
    auto const omega_inv = convert_HEP_to_SI<MassType::dimension_type>(mass) / (abs(charge) * Bmag);
    MomentumVector const plab{rootCS, pLabMag, 0_MeV, 0_MeV};
    auto const Elab = sqrt(plab.getSquaredNorm() + static_pow<2>(mass));
    auto gamma = Elab / mass;
    TimeType const period = 2 * M_PI * omega_inv * gamma;

    Point injectionPos(rootCS, 0_m, 100_m, 0_m);
    stack.addParticle(std::make_tuple(beamCode, calculate_kinetic_energy(plab.getNorm(), get_mass(beamCode)), plab.normalized(), injectionPos, 0_ns));

    // setup relevant processes
    setup::Tracking tracking;

    // put radio processes here
    RadioProcess<decltype(detectorCoREAS), CoREAS<decltype(detectorCoREAS),
            decltype(SimplePropagator(env))>, decltype(SimplePropagator(env))>
            coreas(detectorCoREAS, env);
    output.add("CoREAS", coreas);

    RadioProcess<decltype(detectorZHS), ZHS<decltype(detectorZHS),
            decltype(SimplePropagator(env))>, decltype(SimplePropagator(env))>
            zhs(detectorZHS, env);
    output.add("ZHS", zhs);

    TimeCut cut(period);

    // assemble all processes into an ordered process list
    auto sequence = make_sequence(coreas, zhs, cut);

    // define air shower object, run simulation
    Cascade EAS(env, tracking, sequence, output, stack);
    output.startOfShower();
    EAS.run();
    output.endOfShower();

    CORSIKA_LOG_INFO("|p| = {} and E = {}",plab.getNorm(), Elab);
    CORSIKA_LOG_INFO("period: {}", period);
    CORSIKA_LOG_INFO("gamma: {}", gamma);

    output.endOfLibrary();
}