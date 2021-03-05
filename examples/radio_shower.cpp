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
#include <corsika/media/UniformRefractiveIndex.hpp>
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

template <typename TInterface>
using MyExtraEnv =
    UniformRefractiveIndex<MediumPropertyModel<UniformMagneticField<TInterface>>>;

int main(int argc, char** argv) {

  corsika_logger->set_pattern("[%n:%^%-8l%$] %s:%#: %v");
  logging::set_level(logging::level::trace);

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

  // setup environment (idea 2)
  using EnvType = setup::Environment;
  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  auto builder = make_layered_spherical_atmosphere_builder<
      setup::EnvironmentInterface, MyExtraEnv>::create(center,
                                                       constants::EarthRadius::Mean, 1.,
                                                       Medium::AirDry1Atm,
                                                       MagneticFieldVector{rootCS, 0_T,
                                                                           50_uT, 0_T});
}
