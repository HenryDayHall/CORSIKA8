/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
// #include <catch2/catch.hpp>

#include <corsika/modules/radio/ZHS.hpp>
#include <corsika/modules/radio/CoREAS.hpp>
#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>
#include <corsika/modules/radio/detectors/RadioDetector.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

#include <vector>
#include <xtensor/xtensor.hpp>
#include <xtensor/xbuilder.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xcsv.hpp>
#include <istream>
#include <fstream>
#include <iostream>
#include <math.h>

#include <corsika/media/Environment.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>


#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IRefractiveIndexModel.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ExponentialRefractiveIndex.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/media/UniformMagneticField.hpp>



using namespace corsika;

double constexpr absMargin = 1.0e-7;

template <typename TInterface>
using MyExtraEnv =
UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;


int main() {


    // create a suitable environment ///////////////////////////////////////////////////
    using IModelInterface = IRefractiveIndexModel<IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>>;
    using AtmModel = UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<HomogeneousMedium
        <IModelInterface>>>>;
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
    NuclearComposition const Composition(std::vector<Code>{Code::Nitrogen},
                                         std::vector<float>{1.f});
    // create magnetic field vector
    Vector B1(rootCS, 0_T, 0_T, 0.3809_T);
    // create a Sphere for the medium
    auto Medium = EnvType::createNode<Sphere>(
        center, 1_km * std::numeric_limits<double>::infinity());
    // set the environment properties
    auto const props = Medium->setModelProperties<AtmModel>(ri_, Medium::AirDry1Atm, B1, density, Composition);
    // bind things together
    env.getUniverse()->addChild(std::move(Medium));


    // now create antennas and detectors/////////////////////////////////////////////
    // the antennas location
    const auto point1{Point(rootCS, 30000_m, 0_m, 0_m)};
//    const auto point2{Point(rootCS, 5000_m, 100_m, 0_m)};
//    const auto point3{Point(rootCS, -100_m, -100_m, 0_m)};
//    const auto point4{Point(rootCS, -100_m, 100_m, 0_m)};


    // create times for the antenna
    // 30 km antenna
    const TimeType start{0.994e-4_s};
    const TimeType duration{1.07e-4_s - 0.994e-4_s};
    // 3 km antenna
//    const TimeType start{0.994e-5_s};
//    const TimeType duration{1.7e-5_s - 0.994e-5_s};
    const InverseTimeType sampleRate_{5e+11_Hz};

    std::cout << "number of points in time: " << duration*sampleRate_ << std::endl;

    // create 4 cool antennas
    TimeDomainAntenna ant1("cool antenna", point1, start, duration, sampleRate_);
//    TimeDomainAntenna ant2("cooler antenna", point2, t1, t2, t3);
//    TimeDomainAntenna ant3("coolest antenna", point3, t1, t2, t3);
//    TimeDomainAntenna ant4("No, I am the coolest antenna", point4, t1, t2, t3);

    // construct a radio detector instance to store our antennas
    AntennaCollection<TimeDomainAntenna> detector;

    // add the antennas to the detector
    detector.addAntenna(ant1);
//    detector.addAntenna(ant2);
//    detector.addAntenna(ant3);
//    detector.addAntenna(ant4);

    //////////////////////////////////////////////////////////////////////////////////

    // create a new stack for each trial
    setup::Stack stack;
    stack.clear();

    const Code particle{Code::Electron};
    const HEPMassType pmass{get_mass(particle)};

    // construct an energy // move in the for loop
    const HEPEnergyType E0{11.4_MeV};

    // create a radio process instance using CoREAS
    RadioProcess<decltype(detector), ZHS<decltype(detector), decltype(StraightPropagator(env))>, decltype(StraightPropagator(env))>
        coreas(detector, env);

    // loop over all the tracks except the last one
    int const n_points {100000};
    LengthType const radius {100_m};
    TimeType timeCounter {0._s};
    for (size_t i = 0; i <= (n_points) * 2; i++) {
      Point const point_1(rootCS,{radius*cos(M_PI*2*i/n_points),radius*sin(M_PI*2*i/n_points), 0_m});
      Point const point_2(rootCS,{radius*cos(M_PI*2*(i+1)/n_points),radius*sin(M_PI*2*(i+1)/n_points), 0_m});
      TimeType t {(point_2 - point_1).getNorm() / (0.999 * constants::c)};
      timeCounter = timeCounter + t;
      VelocityVector v { (point_2 - point_1) / t };
      auto  beta {v / constants::c};
      auto gamma {E0/pmass};
      auto plab {beta * pmass * gamma};
      Line l {point_1,v};
      StraightTrajectory track {l,t};
      auto particle1{stack.addParticle(std::make_tuple(particle, E0, plab, point_1, timeCounter))}; //TODO: plab is inconsistent
      coreas.doContinuous(particle1,track,true);
      stack.clear();
    }


    // get the output
    coreas.writeOutput();
}