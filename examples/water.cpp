/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/MediumProperties.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/LongitudinalWriter.hpp>
#include <corsika/modules/writers/EnergyLossWriter.hpp>
#include <corsika/modules/PROPOSAL.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Pythia8.hpp>
#include <corsika/modules/Random.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/UrQMD.hpp>
#include <corsika/modules/tracking/TrackingStraight.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/stack/GeometryNodeStackExtension.hpp>
#include <corsika/stack/WeightStackExtension.hpp>
#include <corsika/stack/VectorStack.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

using namespace corsika;

using IMediumType = IMediumPropertyModel<IMediumModel>;
using EnvType = Environment<IMediumType>;
using StackActive = setup::Stack<EnvType>;
using StackView = StackActive::stack_view_type;
using Particle = StackActive::particle_type;

void registerRandomStreams(int seed) {
  RNGManager<>::getInstance().registerRandomStream("cascade");
  RNGManager<>::getInstance().registerRandomStream("qgsjet");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("epos");
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

int main(int argc, char** argv) {
  // * process input
  Code primaryType;
  HEPEnergyType e0, eCut;
  int A, Z, n_event;
  int randomSeed;
  std::string output_dir;
  CLI::App app{"Neutrino event generator"};
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
  app.add_option("--e0", "Minimum energy [GeV]")
      ->check(CLI::Range(50.0, 1e8))
      ->default_val(1e5)
      ->group("Primary");
  app.add_option("--eCut", "Cut energy [GeV]")->default_val(1.);
  app.add_option("-N,--nevent", n_event, "The number of events/showers to run.")
      ->default_val(1)
      ->check(CLI::PositiveNumber);
  app.add_option("-f,--filename", output_dir, "Filename for output library")
      ->check(CLI::NonexistentPath)
      ->default_val("output");
  app.add_option("-v", "Verbosity level: warn, info, debug, trace.")
      ->default_val("info")
      ->check(CLI::IsMember({"warn", "info", "debug", "trace"}));
  app.add_option("-s", randomSeed, "Seed for random number")
      ->check(CLI::NonNegativeNumber)
      ->default_val(0);
  CLI11_PARSE(app, argc, argv);

  // check that we got either PDG or A/Z
  // this can be done with option_groups but the ordering
  // gets all messed up
  if (app.count("--pdg") == 0) {
    if ((app.count("-A") == 0) || (app.count("-Z") == 0)) {
      CORSIKA_LOG_ERROR("If --pdg is not provided, then both -A and -Z are required.");
      return 1;
    }
  }
  // check if we want to use a PDG code instead
  if (app.count("--pdg") > 0) {
    primaryType = convert_from_PDG(PDGCode(app["--pdg"]->as<int>()));
  } else {
    // check manually for proton and neutrons
    if ((A == 1) && (Z == 1))
      primaryType = Code::Proton;
    else if ((A == 1) && (Z == 0))
      primaryType = Code::Neutron;
    else
      primaryType = get_nucleus_code(A, Z);
  }

  e0 = app["--e0"]->as<double>() * 1_GeV;
  eCut = app["--eCut"]->as<double>() * 1_GeV;
  std::string_view const loglevel = app["-v"]->as<std::string_view>();
  if (loglevel == "warn") {
    logging::set_level(logging::level::warn);
  } else if (loglevel == "info") {
    logging::set_level(logging::level::info);
  } else if (loglevel == "debug") {
    logging::set_level(logging::level::debug);
  } else if (loglevel == "trace") {
#ifndef _C8_DEBUG_
    CORSIKA_LOG_ERROR("trace log level requires a Debug build.");
    return 1;
#endif
    logging::set_level(logging::level::trace);
  }

  registerRandomStreams(randomSeed);

  // * environment and universe
  EnvType env;
  auto& universe = env.getUniverse();
  auto const& rootCS = env.getCoordinateSystem();

  // * Water geometry
  {
    Point const center{rootCS, 0_m, 0_m, 0_m};
    auto sphere = std::make_unique<Sphere>(center, 100_m);
    auto node = std::make_unique<VolumeTreeNode<IMediumType>>(std::move(sphere));
    auto comp = NuclearComposition({{Code::Oxygen}, {1.0}});
    auto density = 1.02_g / (1_cm * 1_cm * 1_cm);
    auto earth_medium =
        std::make_shared<MediumPropertyModel<HomogeneousMedium<IMediumType>>>(
            Medium::StandardRock, density, comp);
    node->setModelProperties(earth_medium);
    universe->addChild(std::move(node));
  }

  // * detector geometry
  auto injectorLength = 50_m;
  Point const injectorPos = Point(rootCS, {0_m, 0_m, injectorLength});
  auto const& injectCS = make_translation(rootCS, injectorPos.getCoordinates());
  DirectionVector upVec(rootCS, {0., 0., 1.});
  DirectionVector leftVec(rootCS, {1., 0., 0.});
  DirectionVector downVec(rootCS, {0., 0., -1.});

  // * observation plane
  std::vector<ObservationPlane<tracking_line::Tracking>> obsPlanes;
  const int nPlane = 5;
  for (int i = 0; i < nPlane - 1; i++) {
    Point planeCenter{injectCS, {0_m, 0_m, -(i + 1) * 3_m}};
    obsPlanes.push_back({Plane(planeCenter, upVec), leftVec, false});
  }
  auto& obsPlaneFinal = obsPlanes.emplace_back(
      Plane{Point{injectCS, {0_m, 0_m, -50_m}}, upVec}, leftVec, true);

  // * longitutional profile
  ShowerAxis const showerAxis{injectorPos, 1.2 * injectorLength * downVec, env};
  LongitudinalWriter longiWriter{showerAxis, 5500, 1_g / square(1_cm)};
  LongitudinalProfile<SubWriter<decltype(longiWriter)>> longprof{longiWriter};

  // * energy loss profile
  EnergyLossWriter dEdX{showerAxis, 1_g / square(1_cm), 5500};

  // * physical process list
  // particle production threshold
  HEPEnergyType const emcut = eCut;
  HEPEnergyType const hadcut = eCut;
  ParticleCut<SubWriter<decltype(dEdX)>> cut(emcut, emcut, hadcut, hadcut, true, dEdX);

  // hadronic interactions
  HEPEnergyType heHadronModelThreshold = 63.1_GeV;
  corsika::sibyll::Interaction sibyll(env);
  // UrQMD not support our nucleon yet. See #456
  corsika::urqmd::UrQMD urqmd;
  InteractionCounter urqmdCounted(urqmd);
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    bool operator()(const Particle& p) const { return (p.getKineticEnergy() < cutE_); }
  };
  // auto lowModel = make_sequence(urqmd);
  auto hadronSequence =
      make_select(EnergySwitch(heHadronModelThreshold), urqmdCounted, sibyll);

  // decay process
  corsika::pythia8::Decay decayPythia;
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
  auto decaySequence = make_sequence(decayPythia, decaySibyll);

  // EM process
  corsika::proposal::Interaction emCascade(env, sibyll.getHadronInteractionModel(),
                                           heHadronModelThreshold);
  corsika::proposal::ContinuousProcess<SubWriter<decltype(dEdX)>> emContinuous(env, dEdX);

  // total physics list
  auto physics_sequence =
      make_sequence(emCascade, emContinuous, hadronSequence, decaySequence);

  // * output module
  OutputManager output(output_dir);
  for (int i = 0; i < nPlane; i++) {
    output.add(fmt::format("particles_{:}", i), obsPlanes[i]);
  }
  // hard coded
  auto obsPlaneSequence =
      make_sequence(obsPlanes[0], obsPlanes[1], obsPlanes[2], obsPlanes[3], obsPlanes[4]);
  output.add("longi_profile", longiWriter);
  output.add("energy_loss", dEdX);

  // * the final process sequence
  auto sequence = make_sequence(physics_sequence, cut, obsPlaneSequence, longprof);

  // * tracking and stack
  tracking_line::Tracking tracking;
  StackActive stack;

  // * cascade manager
  Cascade EAS(env, tracking, sequence, output, stack);

  // * main loop
  output.startOfLibrary();
  for (int i_shower = 0; i_shower < n_event; i_shower++) {
    stack.clear();
    CORSIKA_LOG_INFO("Event: {} / {}", i_shower, n_event);

    // * inject primary
    auto primary = stack.addParticle(std::make_tuple(
        primaryType, e0 - get_mass(primaryType), downVec, injectorPos, 0_ns));

    EAS.run();

    // * report energy loss result
    HEPEnergyType const Efinal = dEdX.getEnergyLost() + obsPlaneFinal.getEnergyGround();
    CORSIKA_LOG_INFO(
        "total energy budget (TeV): {:.2f} (dEdX={:.2f} ground={:.2f}), "
        "relative difference (%): {:.3f}",
        e0 / 1_TeV, dEdX.getEnergyLost() / 1_TeV, obsPlaneFinal.getEnergyGround() / 1_TeV,
        (Efinal / e0 - 1.) * 100.);
  }
  output.endOfLibrary();

  return 0;
}
