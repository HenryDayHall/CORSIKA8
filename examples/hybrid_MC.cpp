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
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/SwitchProcessSequence.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/utility/SaveBoostHistogram.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/core/EnergyMomentumOperations.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/EnergyLossWriter.hpp>
#include <corsika/modules/writers/LongitudinalWriter.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/modules/LongitudinalProfile.hpp>
#include <corsika/modules/ObservationPlane.hpp>
#include <corsika/modules/TrackWriter.hpp>
#include <corsika/modules/ParticleCut.hpp>
#include <corsika/modules/Pythia8.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/UrQMD.hpp>
#include <corsika/modules/PROPOSAL.hpp>
#include <corsika/modules/CONEX.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

/*
  NOTE, WARNING, ATTENTION

  The file Random.hpp implements the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/Random.hpp>

using namespace corsika;
using namespace std;

void registerRandomStreams(uint64_t seed) {
  RNGManager<>::getInstance().registerRandomStream("cascade");
  RNGManager<>::getInstance().registerRandomStream("qgsjet");
  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("epos");
  RNGManager<>::getInstance().registerRandomStream("pythia");
  RNGManager<>::getInstance().registerRandomStream("urqmd");
  RNGManager<>::getInstance().registerRandomStream("proposal");
  RNGManager<>::getInstance().registerRandomStream("conex");
  if (seed == 0) {
    std::random_device rd;
    seed = rd();
    cout << "new random seed (auto) " << seed << endl;
  }
  RNGManager<>::getInstance().setSeed(seed);
}

class TrackCheck : public ContinuousProcess<TrackCheck> {

public:
  TrackCheck(Plane const& plane)
      : plane_(plane) {}

  template <typename TParticle, typename TTrack>
  ProcessReturn doContinuous(TParticle const& particle, TTrack const&, bool const) {
    auto const delta = particle.getPosition() - plane_.getCenter();
    auto const n = plane_.getNormal();
    auto const proj = n.dot(delta);
    if (proj < -1_m) {
      CORSIKA_LOG_INFO("particle {} failes: proj={}, delta={}, p={}", particle.asString(),
                       proj, delta, particle.getPosition());
      throw std::runtime_error("particle below obs level");
    }
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  LengthType getMaxStepLength(TParticle const&, TTrack const&) const {
    return std::numeric_limits<double>::infinity() * 1_m;
  }

private:
  Plane plane_;
};

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

int main(int argc, char** argv) {

  logging::set_level(logging::level::info);

  CORSIKA_LOG_INFO("hybrid_MC");

  if (argc < 4) {
    std::cerr << "usage: hybrid_MC <A> <Z> <energy/GeV> [seed]" << std::endl;
    std::cerr << "       if no seed is given, a random seed is chosen" << std::endl;
    return 1;
  }
  feenableexcept(FE_INVALID);

  uint64_t seed = 0;
  if (argc > 4) seed = std::stol(std::string(argv[4]));
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

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.clear();
  unsigned short const A = std::stoi(std::string(argv[1]));
  unsigned short const Z = std::stoi(std::string(argv[2]));
  Code const beamCode = get_nucleus_code(A, Z);
  auto const mass = get_mass(beamCode);
  HEPEnergyType const E0 = 1_GeV * std::stof(std::string(argv[3]));
  double theta = 0.;
  auto const thetaRad = theta / 180. * M_PI;

  HEPMomentumType P0 = calculate_momentum(E0, mass);
  auto momentumComponents = [](double thetaRad, HEPMomentumType ptot) {
    return std::make_tuple(ptot * sin(thetaRad), 0_eV, -ptot * cos(thetaRad));
  };

  auto const [px, py, pz] = momentumComponents(thetaRad, P0);
  auto plab = MomentumVector(rootCS, {px, py, pz});
  cout << "input particle: " << beamCode << endl;
  cout << "input angles: theta=" << theta << endl;
  cout << "input momentum: " << plab.getComponents() / 1_GeV
       << ", norm = " << plab.getNorm() << endl;

  auto const observationHeight = 0_km + constants::EarthRadius::Mean;
  auto const injectionHeight = 112.75_km + constants::EarthRadius::Mean;
  auto const t = -observationHeight * cos(thetaRad) +
                 sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                      static_pow<2>(injectionHeight));
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
  Point const injectionPos =
      showerCore +
      Vector<dimensionless_d>{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

  std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

  stack.addParticle(std::make_tuple(
      Code::Proton, calculate_kinetic_energy(plab.getNorm(), get_mass(beamCode)),
      plab.normalized(), injectionPos, 0_ns));

  std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.02
            << std::endl;

  OutputManager output("hybrid_MC_outputs");
  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.02, env,
                              false, 1000};

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

  // register energy losses as output
  EnergyLossWriter dEdX{showerAxis, 10_g / square(1_cm), 200};
  output.add("energyloss", dEdX);

  // create a track writer and register it with the output manager
  TrackWriter<TrackWriterParquet> tracks;
  output.add("tracks", tracks);

  ParticleCut<SubWriter<decltype(dEdX)>> cut(3_GeV, false, dEdX);
  BetheBlochPDG<SubWriter<decltype(dEdX)>> eLoss(dEdX);

  LongitudinalWriter profile{showerAxis, 10_g / square(1_cm), 200};
  output.add("profile", profile);
  LongitudinalProfile<SubWriter<decltype(profile)>> longprof{profile};

  CONEXhybrid // SubWriter<decltype(dEdX>, SubWriter<decltype(profile)>>
      conex_model(center, showerAxis, t, injectionHeight, E0, get_PDG(Code::Proton), dEdX,
                  profile);

  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
  ObservationPlane<setup::Tracking> observationLevel(
      obsPlane, DirectionVector(rootCS, {1., 0., 0.}), "particles.dat");
  output.add("obsplane", observationLevel);

  corsika::urqmd::UrQMD urqmd_model;
  InteractionCounter urqmdCounted{urqmd_model};

  // assemble all processes into an ordered process list
  struct EnergySwitch {
    HEPEnergyType cutE_;
    EnergySwitch(HEPEnergyType cutE)
        : cutE_(cutE) {}
    bool operator()(const setup::Stack::particle_type& p) const {
      return (p.getEnergy() < cutE_);
    }
  };
  auto hadronSequence = make_select(EnergySwitch(55_GeV), urqmdCounted,
                                    make_sequence(sibyllNucCounted, sibyllCounted));
  auto decaySequence = make_sequence(decayPythia, decaySibyll);
  auto sequence = make_sequence(hadronSequence, decaySequence, eLoss, cut, conex_model,
                                longprof, observationLevel);

  // define air shower object, run simulation
  setup::Tracking tracking;
  Cascade EAS(env, tracking, sequence, output, stack);

  // to fix the point of first interaction, uncomment the following two lines:
  //  EAS.SetNodes();
  //  EAS.forceInteraction();

  output.startOfShower();
  EAS.run();
  output.endOfShower();

  const HEPEnergyType Efinal = dEdX.getEnergyLost() + observationLevel.getEnergyGround();
  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;

  auto const hists = sibyllCounted.getHistogram() + sibyllNucCounted.getHistogram() +
                     urqmdCounted.getHistogram();

  save_hist(hists.labHist(), "inthist_lab_hybrid.npz", true);
  save_hist(hists.CMSHist(), "inthist_cms_hybrid.npz", true);

  output.endOfLibrary();

  CORSIKA_LOG_INFO("done");
}
