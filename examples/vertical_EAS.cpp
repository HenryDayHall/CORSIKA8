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

#include <corsika/output/OutputManager.hpp>

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

int main(int argc, char** argv) {

  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");
  logging::set_level(logging::level::info);

  CORSIKA_LOG_INFO("vertical_EAS");

  if (argc < 4) {
    std::cerr << "usage: vertical_EAS <A> <Z> <energy/GeV> [seed]" << std::endl;
    std::cerr << "       if no seed is given, a random seed is chosen" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);

  int seed = 0;
  if (argc > 4) seed = std::stoi(std::string(argv[4]));
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

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);
  builder.assemble(env);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  const Code beamCode = Code::Nucleus;
  unsigned short const A = std::stoi(std::string(argv[1]));
  unsigned short Z = std::stoi(std::string(argv[2]));
  auto const mass = get_nucleus_mass(A, Z);
  const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[3]));
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
  auto const injectionHeight = 112.75_km + builder.getEarthRadius();
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

  if (A != 1) {
    stack.addParticle(std::make_tuple(beamCode, E0, plab, injectionPos, 0_ns, A, Z));

  } else {
    if (Z == 1) {
      stack.addParticle(std::make_tuple(Code::Proton, E0, plab, injectionPos, 0_ns));
    } else if (Z == 0) {
      stack.addParticle(std::make_tuple(Code::Neutron, E0, plab, injectionPos, 0_ns));
    } else {
      std::cerr << "illegal parameters" << std::endl;
      return EXIT_FAILURE;
    }
  }

  // we make the axis much longer than the inj-core distance since the
  // profile will go beyond the core, depending on zenith angle
  std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.5
            << std::endl;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.5, env};

  // setup processes, decays and interactions

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

  // create the output manager that we then register outputs with
  OutputManager output("vertical_EAS_outputs");

  ParticleCut cut{60_GeV, 60_GeV, 60_GeV, 60_GeV, true};
  corsika::proposal::Interaction proposal(env);
  corsika::proposal::ContinuousProcess em_continuous(env);
  InteractionCounter proposalCounted(proposal);

  OnShellCheck reset_particle_mass(1.e-3, 1.e-1, false);

  TrackWriter trackWriter;
  output.add("tracks", trackWriter); // register TrackWriter

  LongitudinalProfile longprof{showerAxis};

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane observationLevel(obsPlane, DirectionVector(rootCS, {1., 0., 0.}));

  // register the observation plane with the output
  output.add("obsplane", observationLevel);

  corsika::urqmd::UrQMD urqmd;
  InteractionCounter urqmdCounted{urqmd};
  StackInspector<setup::Stack> stackInspect(1000, false, E0);

  // assemble all processes into an ordered process list
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    SwitchResult operator()(const Particle& p) {
      if (p.getEnergy() < cutE_)
        return SwitchResult::First;
      else
        return SwitchResult::Second;
    }
  };
  auto hadronSequence = make_select(
      urqmdCounted, make_sequence(sibyllNucCounted, sibyllCounted), EnergySwitch(55_GeV));
  auto decaySequence = make_sequence(decayPythia, decaySibyll);
  auto sequence = make_sequence(stackInspect, hadronSequence, reset_particle_mass,
                                decaySequence, proposalCounted, em_continuous, cut,
                                trackWriter, observationLevel, longprof);

  // define air shower object, run simulation
  setup::Tracking tracking;
  Cascade EAS(env, tracking, sequence, output, stack);

  // to fix the point of first interaction, uncomment the following two lines:
  //  EAS.forceInteraction();

  EAS.run();

  cut.showResults();
  em_continuous.showResults();
  observationLevel.showResults();
  const HEPEnergyType Efinal = cut.getCutEnergy() + cut.getInvEnergy() +
                               cut.getEmEnergy() + em_continuous.getEnergyLost() +
                               observationLevel.getEnergyGround();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
  observationLevel.reset();
  cut.reset();
  em_continuous.reset();

  auto const hists = sibyllCounted.getHistogram() + sibyllNucCounted.getHistogram() +
                     urqmdCounted.getHistogram() + proposalCounted.getHistogram();

  save_hist(hists.labHist(), "inthist_lab_verticalEAS.npz", true);
  save_hist(hists.CMSHist(), "inthist_cms_verticalEAS.npz", true);
  longprof.save("longprof_verticalEAS.txt");

  output.endOfLibrary();
}
