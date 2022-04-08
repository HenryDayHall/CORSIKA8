/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define TRACE

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Cascade.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/modules/writers/SubWriter.hpp>
#include <corsika/modules/writers/LongitudinalWriter.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/ShowerAxis.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>


#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

using namespace corsika;
using namespace std;

using EnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
using EnvType = Environment<EnvironmentInterface>;

using Particle = setup::Stack<EnvType>::particle_type;

template <typename T>
using MyExtraEnv = MediumPropertyModel<UniformMagneticField<T>>;

// argv : 1.number of nucleons, 2.number of protons,
//        3.total energy in GeV, 4.number of showers,
//        5.seed (0 by default to generate random values for all)

int main(int argc, char** argv) {

  logging::set_level(logging::level::info);

  // setup environment, geometry
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  
  LengthType observationHeight = 1_m;

  // build a Linsley US Standard atmosphere into `env`
  create_5layer_atmosphere<EnvironmentInterface, MyExtraEnv>(
      env, AtmosphereId::LinsleyUSStd, center, Medium::AirDry1Atm,
      MagneticFieldVector{rootCS, 0_T, 0_T, 2_uT});
  Point const showerCore{rootCS, 0_m, 0_m, observationHeight};  
  Point const injectionPos = showerCore + DirectionVector{rootCS, {0., 0., 1.}} * 10_m;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos) * 1.5, env, false,
                              1000};

  // create the output manager that we then register outputs with
  OutputManager output("vertical_EAS_outputs");

  // setup longitudinal profile
  LongitudinalWriter longProf{showerAxis};
  output.add("profile", longProf);

  LongitudinalProfile<SubWriter<decltype(longProf)>> profile{longProf};

  // setup particle stack, and add primary particle
  setup::Stack<EnvType> stack;

  auto sequence = make_sequence(profile);

  // define air shower object, run simulation
  setup::Tracking tracking;
  Cascade EAS(env, tracking, sequence, output, stack);
  output.startOfShower();
  EAS.run();
  output.endOfShower();
}
