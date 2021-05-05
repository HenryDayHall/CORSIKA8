/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/ShowerAxis.hpp>

#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/modules/BetheBlochPDG.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <fstream>
#include <iostream>
#include <limits>

using namespace corsika;
using namespace std;

//
// This example demonstrates the energy loss of muons as function of beta*gamma (=p/m)
//
int main() {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  CORSIKA_LOG_INFO("stopping_power");

  feenableexcept(FE_INVALID);

  // setup environment, geometry
  using EnvType = Environment<IMediumModel>;
  EnvType env;
  env.getUniverse()->setModelProperties<HomogeneousMedium<IMediumModel>>(
      1_g / cube(1_cm), NuclearComposition{{Code::Unknown}, {1.f}});

  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      112.8_km); // this is the CORSIKA 7 start of atmosphere/universe

  ShowerAxis showerAxis{injectionPos, Vector<length_d>{rootCS, 0_m, 0_m, 1_m}, env};
  BetheBlochPDG eLoss{showerAxis};

  setup::Stack stack;

  std::ofstream file("dEdX.dat");
  file << "# beta*gamma, dE/dX / eV/(g/cm²)" << std::endl;

  for (HEPEnergyType E0 = 300_MeV; E0 < 1_PeV; E0 *= 1.05) {
    stack.clear();
    const Code beamCode = Code::MuPlus;
    const HEPMassType mass = get_mass(beamCode);
    double theta = 0.;
    double phi = 0.;

    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
    };
    HEPMomentumType P0 = elab2plab(E0, mass);
    auto momentumComponents = [](double theta, double phi, HEPMomentumType ptot) {
      return std::make_tuple(ptot * sin(theta) * cos(phi), ptot * sin(theta) * sin(phi),
                             -ptot * cos(theta));
    };
    auto const [px, py, pz] =
        momentumComponents(theta / 180. * M_PI, phi / 180. * M_PI, P0);
    auto plab = MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.getComponents() / 1_GeV << endl;

    stack.addParticle(std::make_tuple(beamCode, plab, injectionPos, 0_ns));

    auto const p = stack.getNextParticle();
    HEPEnergyType dE = eLoss.getTotalEnergyLoss(p, 1_g / square(1_cm));
    file << P0 / mass << "\t" << -dE / 1_eV << std::endl;
  }
}
