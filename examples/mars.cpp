/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/* clang-format off */
// InteractionCounter used boost/histogram, which
// fails if boost/type_traits have been included before. Thus, we have
// to include it first...
#include <corsika/framework/process/InteractionCounter.hpp>
/* clang-format on */
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/EnergyMomentumOperations.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Cascade.hpp>

#include <corsika/framework/utility/SaveBoostHistogram.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/EnergyLossWriter.hpp>
#include <corsika/modules/writers/LongitudinalWriter.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Pythia8.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/UrQMD.hpp>
#include <corsika/modules/PROPOSAL.hpp>
#include <corsika/modules/QGSJetII.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/Random.hpp>

using namespace corsika;
using namespace std;

using EnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
using EnvType = Environment<EnvironmentInterface>;

using Particle = setup::Stack<EnvType>::particle_type;

typedef decltype(1 * pascal) PressureType;
typedef decltype(1 * degree_celsius) TemperatureType;

class MarsAtmModel {
public:
  MarsAtmModel() = delete;
  MarsAtmModel(PressureType a, InverseLengthType b, TemperatureType c,
               decltype(1 * degree_celsius / 1_m) d)
      : a_(a)
      , b_(b)
      , c_(c)
      , d_(d) {}

  MassDensityType operator()(LengthType height) const {
    PressureType const pressure = a_ * exp(-b_ * height);
    TemperatureType const temperature = -c_ - d_ * height + 273.1_K; // in K
    constexpr decltype(square(1_m) / (square(1_s) * 1_K)) constant =
        1000 * 0.1921 * square(1_m) / (square(1_s) * 1_K);
    return pressure / (constant * temperature);
  }

private:
  PressureType a_;
  InverseLengthType b_;
  TemperatureType c_;
  decltype(1_K / 1_m) d_;
};

void registerRandomStreams(int seed) {
  RNGManager<>::getInstance().registerRandomStream("cascade");
  RNGManager<>::getInstance().registerRandomStream("qgsjet");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("pythia");
  RNGManager<>::getInstance().registerRandomStream("urqmd");
  RNGManager<>::getInstance().registerRandomStream("proposal");
  if (seed == 0) {
    std::random_device rd;
    seed = rd();
    CORSIKA_LOG_INFO("random seed (auto) {} ", seed);
  } else {
    CORSIKA_LOG_INFO("random seed {} ", seed);
  }
  RNGManager<>::getInstance().setSeed(seed);
}

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

int main(int argc, char** argv) {

  // the main command line description
  CLI::App app{"Simulate standard (downgoing) showers with CORSIKA 8."};

  // some options that we want to fill in
  int A, Z, nevent = 0;

  // the following section adds the options to the parser

  // we start by definining a sub-group for the primary ID
  auto opt_Z = app.add_option("-Z", Z, "Atomic number for primary")
                   ->check(CLI::Range(0, 26))
                   ->group("Primary");
  auto opt_A = app.add_option("-A", A, "Atomic mass number for primary")
                   ->needs(opt_Z)
                   ->check(CLI::Range(1, 58))
                   ->group("Primary");
  app.add_option("-p,--pdg", "PDG code for primary.")
      ->excludes(opt_A)
      ->excludes(opt_Z)
      ->group("Primary");
  // the remainding options
  app.add_option("-E,--energy", "Primary energy in GeV")
      ->required()
      ->check(CLI::PositiveNumber)
      ->group("Primary");
  app.add_option("-z,--zenith", "Primary zenith angle (deg)")
      ->required()
      ->default_val(0.)
      ->check(CLI::Range(0, 90))
      ->group("Primary");
  app.add_option("-a,--azimuth", "Primary azimuth angle (deg)")
      ->default_val(0.)
      ->check(CLI::Range(0, 360))
      ->group("Primary");
  app.add_option("-N,--nevent", nevent, "The number of events/showers to run.")
      ->required()
      ->check(CLI::PositiveNumber)
      ->group("Library/Output");
  app.add_option("-f,--filename", "Filename for output library.")
      ->required()
      ->default_val("corsika_library")
      ->check(CLI::NonexistentPath)
      ->group("Library/Output");
  app.add_option("-s,--seed", "The random number seed.")
      ->default_val(0)
      ->check(CLI::NonNegativeNumber)
      ->group("Misc.");
  app.add_option("-v,--verbosity", "Verbosity level: warn, info, debug, trace.")
      ->default_val("info")
      ->check(CLI::IsMember({"warn", "info", "debug", "trace"}))
      ->group("Misc.");

  // parse the command line options into the variables
  CLI11_PARSE(app, argc, argv);

  string const loglevel =
      (app.count("--verbosity") ? app["verbosity"]->as<string>() : "info");
  if (loglevel == "warn") {
    logging::set_level(logging::level::warn);
  } else if (loglevel == "info") {
    logging::set_level(logging::level::info);
  } else if (loglevel == "debug") {
    logging::set_level(logging::level::debug);
  } else if (loglevel == "trace") {
#ifndef DEBUG
    CORSIKA_LOG_ERROR("trace log level requires a Debug build.");
    return 1;
#endif
    logging::set_level(logging::level::trace);
  }

  // check that we got either PDG or A/Z
  // this can be done with option_groups but the ordering
  // gets all messed up
  if (app.count("--pdg") == 0) {
    if ((app.count("-A") == 0) || (app.count("-Z") == 0)) {
      CORSIKA_LOG_ERROR("If --pdg is not provided, then both -A and -Z are required.");
      return 1;
    }
  }

  // initialize random number sequence(s)
  registerRandomStreams(app["--seed"]->as<int>());

  /* === START: SETUP ENVIRONMENT AND ROOT COORDINATE SYSTEM === */
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  LengthType const radiusMars = 3389.5_km;
  auto builder =
      make_layered_spherical_atmosphere_builder<EnvironmentInterface, MyExtraEnv>::create(
          center,
          radiusMars,                                   // Mars
          Medium::AirDry1Atm,                           // Mars, close enough
          MagneticFieldVector{rootCS, 0_T, 0_uT, 0_T}); // Mars

  builder.setNuclearComposition(                             // Mars
      {{Code::Nitrogen, Code::Oxygen}, {1. / 3., 2. / 3.}}); // simplified
  //{{Code::Carbon, Code::Oxygen, // 95.97 CO2
  //          Code::Nitrogen},            // 1.89 N2 + 1.93 Argon + 0.146 O2
  //       {0.9597 / 3, 0.9597 * 2 / 3,
  //      1 - 0.9597}}); // values taken from AIRES manual, Ar removed for now

  MarsAtmModel layer1(0.699e3 * pascal, 0.00009 / 1_m, 31.0 * degree_celsius,
                      0.000998 * 1 * degree_celsius / 1_m);
  MarsAtmModel layer2(0.699e3 * pascal, 0.00009 / 1_m, 23.4 * degree_celsius,
                      0.00222 * 1 * degree_celsius / 1_m);

  builder.addTabularLayer(layer1, 100, 100_m, 7_km);
  builder.addTabularLayer(layer2, 300, 500_m, 100_km);
  builder.addLinearLayer(1_g / square(1_cm), 1e9_cm, 112.8_km);
  builder.assemble(env);
  /* === END: SETUP ENVIRONMENT AND ROOT COORDINATE SYSTEM === */

  ofstream atmout("mars.dat");
  for (LengthType h = 0_m; h < 110_km; h += 10_m) {
    Point const ptest{rootCS, 0_m, 0_m, builder.getPlanetRadius() + h};
    auto rho =
        env.getUniverse()->getContainingNode(ptest)->getModelProperties().getMassDensity(
            ptest);
    atmout << h / 1_m << " " << rho / 1_kg * cube(1_m) << "\n";
  }
  atmout.close();

  /* === START: CONSTRUCT PRIMARY PARTICLE === */

  // parse the primary ID as a PDG or A/Z code
  Code beamCode;

  // check if we want to use a PDG code instead
  if (app.count("--pdg") > 0) {
    beamCode = convert_from_PDG(PDGCode(app["--pdg"]->as<int>()));
  } else {
    // check manually for proton and neutrons
    if ((A == 0) && (Z == 1)) beamCode = Code::Proton;
    if ((A == 1) && (Z == 1)) beamCode = Code::Neutron;
  }
  HEPEnergyType const mass = get_mass(beamCode);

  // particle energy
  HEPEnergyType const E0 = 1_GeV * app["--energy"]->as<double>();

  // direction of the shower in (theta, phi) space
  auto const thetaRad = app["--zenith"]->as<double>() / 180. * M_PI;
  auto const phiRad = app["--azimuth"]->as<double>() / 180. * M_PI;

  // convert Elab to Plab
  HEPMomentumType P0 = sqrt((E0 - mass) * (E0 + mass));

  // convert the momentum to the zenith and azimuth angle of the primary
  auto const [px, py, pz] =
      std::make_tuple(P0 * sin(thetaRad) * cos(phiRad), P0 * sin(thetaRad) * sin(phiRad),
                      -P0 * cos(thetaRad));
  auto plab = MomentumVector(rootCS, {px, py, pz});
  /* === END: CONSTRUCT PRIMARY PARTICLE === */

  /* === START: CONSTRUCT GEOMETRY === */
  auto const observationHeight = 0_km + builder.getPlanetRadius();
  auto const injectionHeight = 111.75_km + builder.getPlanetRadius();
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore + DirectionVector{rootCS,
                                   {-sin(thetaRad) * cos(phiRad),
                                    -sin(thetaRad) * sin(phiRad), cos(thetaRad)}} *
                       t;

  // we make the axis much longer than the inj-core distance since the
  // profile will go beyond the core, depending on zenith angle
  /* === END: CONSTRUCT GEOMETRY === */

  // create the output manager that we then register outputs with
  OutputManager output(app["--filename"]->as<std::string>());

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.2, env};

  EnergyLossWriter dEdX{showerAxis};
  output.add("energyloss", dEdX);

  HEPEnergyType const emcut = 1_GeV;
  HEPEnergyType const hadcut = 1_GeV;
  ParticleCut<SubWriter<decltype(dEdX)>> cut(emcut, emcut, hadcut, hadcut, true, dEdX);

  /* === START: SETUP PROCESS LIST === */
  corsika::sibyll::Interaction sibyll;
  InteractionCounter sibyllCounted(sibyll);
  corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  InteractionCounter sibyllNucCounted(sibyllNuc);
  auto heModelCounted = make_sequence(sibyllNucCounted, sibyllCounted);

  corsika::pythia8::Decay decayPythia;

  // use sibyll decay routine for decays of particles unknown to pythia
  corsika::sibyll::Decay decaySibyll{{
      Code::N1440Plus,
      Code::N1440MinusBar,
      Code::N1440_0,
      Code::N1440_0Bar,
      Code::N1710Plus,
      Code::N1710MinusBar,
      Code::N1710_0,
      Code::N1710_0Bar,

      Code::Pi1300Plus,
      Code::Pi1300Minus,
      Code::Pi1300_0,

      Code::KStar0_1430_0,
      Code::KStar0_1430_0Bar,
      Code::KStar0_1430_Plus,
      Code::KStar0_1430_MinusBar,
  }};

  // decaySibyll.printDecayConfig();

  // energy threshold for high energy hadronic model. Affects LE/HE switch for hadron
  // interactions and the hadronic photon model in proposal
  HEPEnergyType heHadronModelThreshold = 63.1_GeV;

  corsika::proposal::Interaction emCascade(env, sibyll, heHadronModelThreshold);
  // NOT possible right now, due to interface difference for PROPOSAL:
  //  InteractionCounter emCascadeCounted(emCascade);
  // corsika::proposal::ContinuousProcess<SubWriter<decltype(dEdX)>>
  // emContinuous(env,dEdX);
  BetheBlochPDG<SubWriter<decltype(dEdX)>> emContinuous{dEdX};

  LongitudinalWriter longprof{showerAxis};
  output.add("profile", longprof);
  LongitudinalProfile<SubWriter<decltype(longprof)>> profile{longprof};

  corsika::urqmd::UrQMD urqmd;
  InteractionCounter urqmdCounted{urqmd};
  StackInspector<setup::Stack<EnvType>> stackInspect(5000, false, E0);

  // assemble all processes into an ordered process list
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    bool operator()(Particle const& p) const { return (p.getKineticEnergy() < cutE_); }
  };
  auto hadronSequence =
      make_select(EnergySwitch(heHadronModelThreshold), urqmdCounted, heModelCounted);
  auto decaySequence = make_sequence(decayPythia, decaySibyll);

  // track writer
  TrackWriter trackWriter;
  output.add("tracks", trackWriter); // register TrackWriter

  // observation plane
  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane<setup::Tracking> observationLevel(
      obsPlane, DirectionVector(rootCS, {1., 0., 0.}));
  // register the observation plane with the output
  output.add("particles", observationLevel);

  // assemble the final process sequence
  auto sequence =
      make_sequence(stackInspect, hadronSequence, decaySequence, emCascade, emContinuous,
                    cut, trackWriter, observationLevel, profile);
  /* === END: SETUP PROCESS LIST === */

  // create the cascade object using the default stack and tracking implementation
  setup::Tracking tracking;
  setup::Stack<EnvType> stack;
  Cascade EAS(env, tracking, sequence, output, stack);

  // print our primary parameters all in one place
  if (app["--pdg"]->count() > 0) {
    CORSIKA_LOG_INFO("Primary PDG ID: {}", app["--pdg"]->as<int>());
  } else {
    CORSIKA_LOG_INFO("Primary Z/A: {}/{}", Z, A);
  }
  CORSIKA_LOG_INFO("Primary Energy: {}", E0);
  CORSIKA_LOG_INFO("Primary Momentum: {}", P0);
  CORSIKA_LOG_INFO("Point of Injection: {}", injectionPos.getCoordinates());
  CORSIKA_LOG_INFO("Shower Axis Length: {}", (showerCore - injectionPos).getNorm() * 1.2);

  // trigger the output manager to open the library for writing
  output.startOfLibrary();

  // loop over each shower
  for (int i_shower = 1; i_shower < nevent + 1; i_shower++) {
    CORSIKA_LOG_INFO("Shower {} / {} ", i_shower, nevent);

    // trigger the start of the outputs for this shower
    output.startOfShower();

    // directory for outputs
    string const outdir(app["--filename"]->as<std::string>());
    string const labHist_file = outdir + "/inthist_lab_" + to_string(i_shower) + ".npz";
    string const cMSHist_file = outdir + "/inthist_cms_" + to_string(i_shower) + ".npz";

    // setup particle stack, and add primary particle
    stack.clear();

    // add the desired particle to the stack
    stack.addParticle(std::make_tuple(
        beamCode, calculate_kinetic_energy(plab.getNorm(), get_mass(beamCode)),
        plab.normalized(), injectionPos, 0_ns));

    // run the shower
    EAS.run();

    HEPEnergyType const Efinal =
        dEdX.getEnergyLost() + observationLevel.getEnergyGround();

    CORSIKA_LOG_INFO(
        "total energy budget (GeV): {}, "
        "relative difference (%): {}",
        Efinal / 1_GeV, (Efinal / E0 - 1) * 100);

    auto const hists = sibyllCounted.getHistogram() + sibyllNucCounted.getHistogram() +
                       urqmdCounted.getHistogram();

    save_hist(hists.labHist(), labHist_file, true);
    save_hist(hists.CMSHist(), cMSHist_file, true);

    // trigger the output manager to save this shower to disk
    output.endOfShower();
  }

  // and finalize the output on disk
  output.endOfLibrary();
}
