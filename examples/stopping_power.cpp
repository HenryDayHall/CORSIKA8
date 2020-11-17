/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/Environment.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <corsika/modules/BetheBlochPDG.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <fstream>
#include <iostream>
#include <limits>

using namespace corsika;
using namespace corsika::units::si;
using namespace std;

//
// This example demonstrates the energy loss of muons as function of beta*gamma (=p/m)
//
int main() {
  feenableexcept(FE_INVALID);

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  Point const injectionPos(
      rootCS, 0_m, 0_m,
      112.8_km); // this is the CORSIKA 7 start of atmosphere/universe

  Vector<dimensionless_d> showerAxis(rootCS, {0, 0, -1});

  corsika::energy_loss::BetheBlochPDG eLoss(injectionPos, showerAxis);

  setup::Stack stack;

  std::ofstream file("dEdX.dat");
  file << "# beta*gamma, dE/dX / eV/(g/cm²)" << std::endl;

  for (HEPEnergyType E0 = 300_MeV; E0 < 1_PeV; E0 *= 1.05) {
    stack.Clear();
    const Code beamCode = Code::MuPlus;
    const HEPMassType mass = corsika::mass(beamCode);
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
    auto plab = corsika::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;

    stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{beamCode, E0, plab, injectionPos,
                                                        0_ns});

    auto const p = stack.GetNextParticle();
    HEPEnergyType dE = eLoss.TotalEnergyLoss(p, 1_g / square(1_cm));
    file << P0 / mass << "\t" << -dE / 1_eV << std::endl;
  }
}
