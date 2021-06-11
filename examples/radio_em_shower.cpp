/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/utility/SaveBoostHistogram.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/framework/process/InteractionCounter.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/output/OutputManager.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>
/* #include <corsika/modules/ObservationPlane.hpp> */
#include <corsika/modules/ParticleCut.hpp>
/* #include <corsika/modules/TrackWriter.hpp> */
#include <corsika/modules/PROPOSAL.hpp>

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

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>

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

void registerRandomStreams() {
    RNGManager::getInstance().registerRandomStream("cascade");
    RNGManager::getInstance().registerRandomStream("proposal");
    RNGManager::getInstance().seedAll();
}

template <typename TInterface>
using MyExtraEnv =
UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;

int main(int argc, char** argv) {

    logging::set_level(logging::level::info);

    if (argc != 2) {
        std::cerr << "usage: em_shower <energy/GeV>" << std::endl;
        return 1;
    }
    feenableexcept(FE_INVALID);
    // initialize random number sequence(s)
    registerRandomStreams();

    // setup environment, geometry
    using EnvType = setup::Environment;
    EnvType env;
    CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
    Point const center{rootCS, 0_m, 0_m, 0_m};
    auto builder = make_layered_spherical_atmosphere_builder<
            setup::EnvironmentInterface, MyExtraEnv>::create(center,
                                                             constants::EarthRadius::Mean, 1.000327,
                                                             Medium::AirDry1Atm,
                                                             MagneticFieldVector{rootCS, 50_uT,
                                                                                 0_T, 0_T});

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

    // setup particle stack, and add primary particle
    setup::Stack stack;
    stack.clear();
    const Code beamCode = Code::Electron;
    auto const mass = get_mass(beamCode);
    const HEPEnergyType E0 = 1_GeV * std::stof(std::string(argv[1]));
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

    auto const observationHeight = 1.4_km + builder.getEarthRadius();
    auto const injectionHeight = 112.75_km + builder.getEarthRadius();
    auto const t = -observationHeight * cos(thetaRad) +
                   sqrt(-static_pow<2>(sin(thetaRad) * observationHeight) +
                        static_pow<2>(injectionHeight));
    Point const showerCore{rootCS, 0_m, 0_m, observationHeight};
    Point const injectionPos =
            showerCore + DirectionVector{rootCS, {-sin(thetaRad), 0, cos(thetaRad)}} * t;

    std::cout << "point of injection: " << injectionPos.getCoordinates() << std::endl;

    stack.addParticle(std::make_tuple(beamCode, plab, injectionPos, 0_ns));

    std::cout << "shower axis length: " << (showerCore - injectionPos).getNorm() * 1.02
              << std::endl;

    OutputManager output("radio_em_shower_outputs");
    ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.02, env};

    // the antenna time variables
    const TimeType duration_{1e-6_s};
    const InverseTimeType sampleRate_{1e+9_Hz};

    // the detector (aka antenna collection) for CoREAS and ZHS
    AntennaCollection<TimeDomainAntenna> detectorCoREAS;
    AntennaCollection<TimeDomainAntenna> detectorZHS;

    auto const showerCoreX_ {showerCore.getCoordinates().getX()};
    auto const showerCoreY_ {showerCore.getCoordinates().getY()};
    auto const injectionPosX_ {injectionPos.getCoordinates().getX()};
    auto const injectionPosY_ {injectionPos.getCoordinates().getY()};
    auto const injectionPosZ_ {injectionPos.getCoordinates().getZ()};
    auto const triggerpoint_ {Point(rootCS, injectionPosX_, injectionPosY_, injectionPosZ_)};
    std::cout << "Trigger Point is: " << triggerpoint_ << std::endl;

//  // setup CoREAS antennas
//  for (auto radius_ = 100_m; radius_ <= 200_m; radius_ += 100_m) {
//    for (auto phi_ = 0; phi_ <= 315; phi_ += 45) {
//      auto phiRad_ = phi_ / 180. * M_PI;
//      auto const point_ {Point(rootCS, showerCoreX_ + radius_ * cos(phiRad_), showerCoreY_ + radius_ * sin(phiRad_), builder.getEarthRadius())};
//      auto triggertime_ {(triggerpoint_ - point_).getNorm() / constants::c};
//      const int rr_ = static_cast<int>(radius_ / 1_m);
//      std::string name_ = "CoREAS_R=" + std::to_string(rr_) + "_m--Phi=" + std::to_string(phi_) + "degrees";
//      TimeDomainAntenna antenna_(name_, point_, triggertime_, duration_, sampleRate_);
//      detectorCoREAS.addAntenna(antenna_);
//    }
//  }
//
//  // setup ZHS antennas
//  for (auto radius_ = 100_m; radius_ <= 200_m; radius_ += 100_m) {
//    for (auto phi_ = 0; phi_ <= 315; phi_ += 45) {
//      auto phiRad_ = phi_ / 180. * M_PI;
//      auto const point_ {Point(rootCS, showerCoreX_ + radius_ * cos(phiRad_), showerCoreY_ + radius_ * sin(phiRad_), builder.getEarthRadius())};
//      auto triggertime_ {(triggerpoint_ - point_).getNorm() / constants::c};
//      const int rr_ = static_cast<int>(radius_ / 1_m);
//      std::string name_ = "ZHS_R=" + std::to_string(rr_) + "_m--Phi=" + std::to_string(phi_) + "degrees";
//      TimeDomainAntenna antenna_(name_, point_, triggertime_, duration_, sampleRate_);
//      detectorZHS.addAntenna(antenna_);
//    }
//  }

    // 4 dummy antennas for CoREAS
    for (auto radius_ = 100_m; radius_ <= 200_m; radius_ += 100_m) {
        for (auto phi_ = 0; phi_ <= 270; phi_ += 90) {
            auto phiRad_ = phi_ / 180. * M_PI;
            auto const point_{
                    Point(rootCS, showerCoreX_ + radius_ * cos(phiRad_), showerCoreY_ + radius_ * sin(phiRad_),
                          builder.getEarthRadius())};
            auto triggertime_{(triggerpoint_ - point_).getNorm() / constants::c};
            const int rr_ = static_cast<int>(radius_ / 1_m);
            std::string name_ = "CoREAS_R=" + std::to_string(rr_) + "_m--Phi=" + std::to_string(phi_) + "degrees";
            TimeDomainAntenna antenna_(name_, point_, triggertime_, duration_, sampleRate_);
            detectorCoREAS.addAntenna(antenna_);
        }
    }

    // 4 dummy antennas for ZHS
    for (auto radius_ = 100_m; radius_ <= 200_m; radius_ += 100_m) {
        for (auto phi_ = 0; phi_ <= 270; phi_ += 90) {
            auto phiRad_ = phi_ / 180. * M_PI;
            auto const point_{
                    Point(rootCS, showerCoreX_ + radius_ * cos(phiRad_), showerCoreY_ + radius_ * sin(phiRad_),
                          builder.getEarthRadius())};
            auto triggertime_{(triggerpoint_ - point_).getNorm() / constants::c};
            const int rr_ = static_cast<int>(radius_ / 1_m);
            std::string name_ = "ZHS_R=" + std::to_string(rr_) + "_m--Phi=" + std::to_string(phi_) + "degrees";
            TimeDomainAntenna antenna_(name_, point_, triggertime_, duration_, sampleRate_);
            detectorZHS.addAntenna(antenna_);
        }
    }

    // setup processes, decays and interactions

    ParticleCut cut(5_MeV, 5_MeV, 100_GeV, 100_GeV, true);
    corsika::proposal::Interaction emCascade(env);
    corsika::proposal::ContinuousProcess emContinuous(env);
    InteractionCounter emCascadeCounted(emCascade);

//  TrackWriter trackWriter;
//  output.add("tracks", trackWriter); // register TrackWriter

    // long. profile; columns for photon, e+, e- still need to be added
    LongitudinalProfile longprof{showerAxis};


    // initiate CoREAS
    RadioProcess<decltype(detectorCoREAS), CoREAS<decltype(detectorCoREAS),
            decltype(SimplePropagator(env))>, decltype(SimplePropagator(env))>
                                              coreas(detectorCoREAS, env);

    // register CoREAS with the output manager
    output.add("CoREAS", coreas);

    // initiate ZHS
    RadioProcess<decltype(detectorZHS), ZHS<decltype(detectorZHS),
            decltype(SimplePropagator(env))>, decltype(SimplePropagator(env))>
                                              zhs(detectorZHS, env);

    // register ZHS with the output manager
    output.add("ZHS", zhs);


//  Plane const obsPlane(showerCore, DirectionVector(rootCS, {0., 0., 1.}));
//  ObservationPlane observationLevel(obsPlane, DirectionVector(rootCS, {1., 0., 0.}),
//                                    "particles.dat");
//  output.add("obsplane", observationLevel);

    auto sequence = make_sequence(emCascadeCounted, emContinuous, cut, longprof, coreas, zhs);
    // ,observationLevel, trackWriter);
    // define air shower object, run simulation
    setup::Tracking tracking;
    Cascade EAS(env, tracking, sequence, output, stack);

    // to fix the point of first interaction, uncomment the following two lines:
    //  EAS.setNodes();
    //  EAS.forceInteraction();

    EAS.run();

    cut.showResults();
    emContinuous.showResults();
//  observationLevel.showResults();
//  const HEPEnergyType Efinal = cut.getCutEnergy() + cut.getInvEnergy() +
//                               cut.getEmEnergy() + emContinuous.getEnergyLost() +
//                               observationLevel.getEnergyGround();
//  cout << "total cut energy (GeV): " << Efinal / 1_GeV << endl
//       << "relative difference (%): " << (Efinal / E0 - 1) * 100 << endl;
//  observationLevel.reset();
    cut.reset();
    emContinuous.reset();

    auto const hists = emCascadeCounted.getHistogram();
    save_hist(hists.labHist(), "inthist_lab_emShower.npz", true);
    save_hist(hists.CMSHist(), "inthist_cms_emShower.npz", true);
    longprof.save("longprof_emShower.txt");

    output.endOfLibrary();
}
