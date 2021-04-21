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
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/utility/SaveBoostHistogram.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

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
#include <corsika/modules/OnShellCheck.hpp>
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

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpppp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>
#include <corsika/modules/qgsjetII/Random.hpp>

using namespace corsika;
using namespace std;

using Particle = setup::Stack::particle_type;

void registerRandomStreams(const int seed) {
  RNGManager::getInstance().registerRandomStream("cascade");
  RNGManager::getInstance().registerRandomStream("qgsjet");
  RNGManager::getInstance().registerRandomStream("sibyll");
  RNGManager::getInstance().registerRandomStream("pythia");
  RNGManager::getInstance().registerRandomStream("urqmd");
  RNGManager::getInstance().registerRandomStream("proposal");

  if (seed == 0)
    RNGManager::getInstance().seedAll();
  else
    RNGManager::getInstance().seedAll(seed);
}

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

// argv : 1.number of nucleons, 2.number of protons,
//        3.total energy in GeV, 4.number of showers,
//        5.seed (0 by default to generate random values for all)

int main(int argc, char** argv) {

  corsika_logger->set_pattern("[%n:%^%-8l%$] %s:%#: %v");
  logging::set_level(logging::level::info);

  CORSIKA_LOG_INFO("vertical_EAS");

  if (argc < 5) {
    std::cerr << "usage: vertical_EAS <A> <Z> <energy/GeV> <Nevt> [seed] [output-dir] \n"
                 "       if A=0, Z is interpreted as PDG code \n"
                 "       if no seed is given, a random seed is chosen \n"
              << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);

  int seed = 0;
  int number_showers = std::stoi(std::string(argv[4]));

  if (argc > 5) { seed = std::stoi(std::string(argv[5])); }

  CORSIKA_LOG_INFO("output_dir={} seed={}", output_dir, seed);

  // initialize random number sequence(s)
  registerRandomStreams(seed);

  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  auto builder = make_layered_spherical_atmosphere_builder<
      setup::EnvironmentInterface, MyExtraEnv>::create(center,
                                                       constants::EarthRadius::Mean,
                                                       Medium::AirDry1Atm,
                                                       MagneticFieldVector{rootCS, 0_T,
                                                                           50_uT, 0_T});
  builder.setNuclearComposition(
      {{Code::Nitrogen, Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 2_km);
  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km + constants::EarthRadius::Mean);
  builder.assemble(env);

  CORSIKA_LOG_DEBUG(
      "environment setup: universe={}, layer1={}, layer2={}, layer3={}, layer4={}, "
      "layer5={}",
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 130000_km, 0_m, 0_m}))),
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 110_km, 0_m, 0_m}))),
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 50_km, 0_m, 0_m}))),
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 20_km, 0_m, 0_m}))),
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 5_km, 0_m, 0_m}))),
      fmt::ptr(env.getUniverse()->getContainingNode(
          Point(rootCS, {constants::EarthRadius::Mean + 2_km, 0_m, 0_m}))));

  // pre-setup particle stack
  unsigned short const A = std::stoi(std::string(argv[1]));
  Code beamCode;
  HEPEnergyType mass;
  unsigned short Z = 0;
  if (A > 0) {
    beamCode = Code::Nucleus;
    Z = std::stoi(std::string(argv[2]));
    mass = get_nucleus_mass(A, Z);
  } else {
    int pdg = std::stoi(std::string(argv[2]));
    beamCode = convert_from_PDG(PDGCode(pdg));
    mass = get_mass(beamCode);
  }
  HEPEnergyType const E0 = 1_GeV * std::stof(std::string(argv[3]));
  double theta = 20.;
  double phi = 180.;
  auto const thetaRad = theta / 180. * M_PI;
  auto const phiRad = phi / 180. * M_PI;

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };
  HEPMomentumType P0 = elab2plab(E0, mass);
  auto momentumComponents = [](double theta, double phi, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(theta) * cos(phi), ptot * sin(theta) * sin(phi),
                           -ptot * cos(theta));
  };

  auto const [px, py, pz] = momentumComponents(thetaRad, phiRad, P0);
  auto plab = MomentumVector(rootCS, {px, py, pz});
  cout << "input particle: " << beamCode << endl;
  cout << "input angles: theta=" << theta << ",phi=" << phi << endl;
  cout << "input momentum: " << plab.getComponents() / 1_GeV
       << ", norm = " << plab.getNorm() << endl;

  auto const observationHeight = 0_km + builder.getEarthRadius();
  auto const injectionHeight = 111.75_km + builder.getEarthRadius();
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore + DirectionVector{rootCS,
                                   {-sin(thetaRad) * cos(phiRad),
                                    -sin(thetaRad) * sin(phiRad), cos(thetaRad)}} *
                       t;

  std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

  // we make the axis much longer than the inj-core distance since the
  // profile will go beyond the core, depending on zenith angle
  std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.5
            << std::endl;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.5, env};

  // setup processes, decays and interactions

  // corsika::qgsjetII::Interaction qgsjet;
  corsika::sibyll::Interaction sibyll;
  InteractionCounter sibyllCounted(sibyll);

  corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
  InteractionCounter sibyllNucCounted(sibyllNuc);

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

  decaySibyll.printDecayConfig();

  ParticleCut cut{60_GeV, 60_GeV, 60_GeV, 60_GeV, true};
  corsika::proposal::Interaction emCascade(env);
  corsika::proposal::ContinuousProcess emContinuous(env);
  InteractionCounter emCascadeCounted(emCascade);

  OnShellCheck reset_particle_mass(1.e-3, 1.e-1, false);

  LongitudinalProfile longprof{showerAxis};

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));

  corsika::urqmd::UrQMD urqmd;
  InteractionCounter urqmdCounted{urqmd};
  StackInspector<setup::Stack> stackInspect(50000, false, E0);

  // assemble all processes into an ordered process list
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    bool operator()(const Particle& p) { return (p.getEnergy() < cutE_); }
  };
  auto hadronSequence = make_select(EnergySwitch(55_GeV), urqmdCounted,
                                    make_sequence(sibyllNucCounted, sibyllCounted));
  auto decaySequence = make_sequence(decayPythia, decaySibyll);

  for (int i_shower = 1; i_shower < number_showers + 1; i_shower++) {

    // directory for outputs
    string const labHist_file = "inthist_lab_verticalEAS_" + to_string(i_shower) + ".npz";
    string const cMSHist_file = "inthist_cms_verticalEAS_" + to_string(i_shower) + ".npz";
    string const longprof_file = "longprof_verticalEAS_" + to_string(i_shower) + ".txt";
    string const tracks_file = "tracks_" + to_string(i_shower) + ".dat";
    string const particles_file = "particles_" + to_string(i_shower) + ".dat";

    std::cout << std::endl;
    std::cout << "Shower " << i_shower << "/" << number_showers << std::endl;

    // setup particle stack, and add primary particle
    setup::Stack stack;
    stack.clear();
    unsigned short const A = std::stoi(std::string(argv[1]));
    Code beamCode;
    HEPEnergyType mass;
    unsigned short Z = 0;
    if (A > 0) {
      beamCode = Code::Nucleus;
      Z = std::stoi(std::string(argv[2]));
      mass = get_nucleus_mass(A, Z);
    } else {
      int pdg = std::stoi(std::string(argv[2]));
      beamCode = convert_from_PDG(PDGCode(pdg));
      mass = get_mass(beamCode);
    }
    HEPEnergyType const E0 = 1_GeV * std::stof(std::string(argv[3]));
    double theta = 0.;
    auto const thetaRad = theta / 180. * M_PI;

    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
    };
    HEPMomentumType P0 = elab2plab(E0, mass);
    auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
      return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
    };

    auto const [px, py, pz] = momentumComponents(thetaRad, P0);
    auto plab = MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << endl;
    cout << "input momentum: " << plab.getComponents() / 1_GeV
         << ", norm = " << plab.getNorm() << endl;

    auto const observationHeight = 0_km + builder.getEarthRadius();
    auto const injectionHeight = 111.75_km + builder.getEarthRadius();
    auto const t = (injectionHeight - observationHeight) / cos(thetaRad);
    Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
    Point const injectionPos =
        showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

    std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

    if (A > 1) {
      stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns, A, Z));

    } else {
      if (A == 1) {
        if (Z == 1) {
          stack.addParticle(std::make_tuple(Code::Proton, E0, plab, injectionPos, 0_ns));
        } else if (Z == 0) {
          stack.addParticle(std::make_tuple(Code::Neutron, E0, plab, injectionPos, 0_ns));
        } else {
          std::cerr << "illegal parameters" << std::endl;
          return EXIT_FAILURE;
        }
      } else {
        stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns));
      }
    }

    // we make the axis much longer than the inj-core distance since the
    // profile will go beyond the core, depending on zenith angle
    std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.5
              << std::endl;

    ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.5, env,
                                false};

    // setup processes, decays and interactions

    // corsika::qgsjetII::Interaction qgsjet;
    corsika::sibyll::Interaction sibyll;
    InteractionCounter sibyllCounted(sibyll);

    corsika::sibyll::NuclearInteraction sibyllNuc(sibyll, env);
    InteractionCounter sibyllNucCounted(sibyllNuc);

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

    decaySibyll.printDecayConfig();

    ParticleCut cut{60_GeV, true, true};
    // corsika::proposal::Interaction emCascade(env);
    // corsika::proposal::ContinuousProcess emContinuous(env);
    // InteractionCounter emCascadeCounted(emCascade);
    BetheBlochPDG emContinuous(showerAxis);

    OnShellCheck reset_particle_mass(1.e-3, 1.e-1, false);
    TrackWriter trackWriter(tracks_dir);

    LongitudinalProfile longprof{showerAxis};

    Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
    ObservationPlane observationLevel(obsPlane, DirectionVector(rootCS, {1., 0., 0.}),
                                      particles_file);

    auto sequence =
        make_sequence(stackInspect, hadronSequence, reset_particle_mass, decaySequence,
                      emContinuous, cut, trackWriter, observationLevel, longprof);

    // define air shower object, run simulation
    setup::Tracking tracking;
    Cascade EAS(env, tracking, sequence, stack);

    // to fix the point of first interaction, uncomment the following two lines:
    //  EAS.forceInteraction();

    EAS.run();

    cut.showResults();
    // emContinuous.showResults();
    observationLevel.showResults();
    const HEPEnergyType Efinal = cut.getCutEnergy() + cut.getInvEnergy() +
                                 cut.getEmEnergy() + // emContinuous.getEnergyLost() +
                                 observationLevel.getEnergyGround();
    cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
         << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
    observationLevel.reset();
    cut.reset();
    // emContinuous.reset();

    longprof.save(longprof_dat);

    auto const hists = sibyllCounted.getHistogram() + sibyllNucCounted.getHistogram() +
                       urqmdCounted.getHistogram();

    save_hist(hists.labHist(), labHist_file, true);
    save_hist(hists.CMSHist(), cMSHist_file, true);
    longprof.save(longprof_file);
  }
}
