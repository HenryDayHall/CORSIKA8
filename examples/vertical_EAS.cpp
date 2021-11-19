/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define TRACE

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
#include <corsika/output/NoOutput.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Pythia8.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/UrQMD.hpp>
#include <corsika/modules/Epos.hpp>

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
#include <corsika/modules/Random.hpp>

using namespace corsika;
using namespace std;

using Particle = setup::Stack::particle_type;

void registerRandomStreams(int seed) {
  RNGManager<>::getInstance().registerRandomStream("cascade");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("pythia");
  RNGManager<>::getInstance().registerRandomStream("urqmd");
  RNGManager<>::getInstance().registerRandomStream("proposal");
  if (seed == 0) {
    std::random_device rd;
    seed = rd();
    cout << "new random seed (auto) " << seed << endl;
  }
  RNGManager<>::getInstance().setSeed(seed);
}

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

// argv : 1.number of nucleons, 2.number of protons,
//        3.total energy in GeV, 4.number of showers,
//        5.seed (0 by default to generate random values for all)

int main(int argc, char** argv) {

  logging::set_level(logging::level::info);

  CORSIKA_LOG_INFO("vertical_EAS");

  if (argc < 4) {
    std::cerr << "usage: vertical_EAS <A> <Z> <energy/GeV> [seed] \n"
                 "       if A=0, Z is interpreted as PDG code \n"
                 "       if no seed is given, a random seed is chosen \n"
              << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);

  int seed = 0;

  if (argc > 4) { seed = std::stoi(std::string(argv[4])); }

  // initialize random number sequence(s)
  registerRandomStreams(seed);

  // setup environment, geometry
  using EnvType = setup::Environment;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};

  // build a Linsley US Standard atmosphere into `env`
  create_5layer_atmosphere<setup::EnvironmentInterface, MyExtraEnv>(
      env, AtmosphereId::LinsleyUSStd, center, Medium::AirDry1Atm,
      MagneticFieldVector{rootCS, 0_T, 50_uT, 0_T});

  // pre-setup particle stack
  unsigned short const A = std::stoi(std::string(argv[1]));
  Code beamCode;
  unsigned short Z = 0;
  if (A > 0) {
    Z = std::stoi(std::string(argv[2]));
    if (A == 1 && Z == 0)
      beamCode = Code::Neutron;
    else if (A == 1 && Z == 1)
      beamCode = Code::Proton;
    else
      beamCode = get_nucleus_code(A, Z);
  } else {
    int pdg = std::stoi(std::string(argv[2]));
    beamCode = convert_from_PDG(PDGCode(pdg));
  }
  HEPEnergyType mass = get_mass(beamCode);
  HEPEnergyType const E0 = 1_GeV * std::stof(std::string(argv[3]));
  double theta = 0.;
  double phi = 180.;
  auto const thetaRad = theta / 180. * constants::pi;
  auto const phiRad = phi / 180. * constants::pi;

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

  auto const observationHeight = 0_km + constants::EarthRadius::Mean;
  auto const injectionHeight = 111.75_km + constants::EarthRadius::Mean;
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

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.5, env, false,
                              1000};

  // create the output manager that we then register outputs with
  OutputManager output("vertical_EAS_outputs");

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

  ParticleCut cut{60_GeV, 60_GeV, 60_GeV, 60_GeV, true};

  corsika::urqmd::UrQMD urqmd;
  InteractionCounter urqmdCounted{urqmd};
  StackInspector<setup::Stack> stackInspect(50000, false, E0);

  // assemble all processes into an ordered process list
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    bool operator()(const Particle& p) const { return (p.getEnergy() < cutE_); }
  };
  auto hadronSequence = make_select(EnergySwitch(55_GeV), urqmdCounted,
                                    make_sequence(sibyllNucCounted, sibyllCounted));
  auto decaySequence = make_sequence(decayPythia, decaySibyll);

  // directory for outputs
  string const labHist_file = "inthist_lab_verticalEAS.npz";
  string const cMSHist_file = "inthist_cms_verticalEAS.npz";
  string const longprof_file = "longprof_verticalEAS.txt";

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();

  stack.addParticle(std::make_tuple(beamCode, plab, injectionPos, 0_ns));

  BetheBlochPDG emContinuous(showerAxis);

  TrackWriter trackWriter;
  output.add("tracks", trackWriter); // register TrackWriter

  LongitudinalProfile longprof{showerAxis};

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane<setup::Tracking, NoOutput> observationLevel(
      obsPlane, DirectionVector(rootCS, {1., 0., 0.}));
  // register the observation plane with the output
  output.add("particles", observationLevel);

  auto sequence = make_sequence(stackInspect, hadronSequence, decaySequence, emContinuous,
                                cut, trackWriter, observationLevel, longprof);

  // define air shower object, run simulation
  setup::Tracking tracking;
  Cascade EAS(env, tracking, sequence, output, stack);
  output.startOfShower();
  EAS.run();
  output.endOfShower();

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

  auto const hists = sibyllCounted.getHistogram() + sibyllNucCounted.getHistogram() +
                     urqmdCounted.getHistogram();

  save_hist(hists.labHist(), labHist_file, true);
  save_hist(hists.CMSHist(), cMSHist_file, true);
  longprof.save(longprof_file);

  output.endOfLibrary();
}
