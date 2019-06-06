/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/process/energy_loss/EnergyLoss.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/CorsikaFenv.h>

#include <fstream>
#include <iostream>
#include <limits>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::particles;
using namespace corsika::geometry;
using namespace corsika::environment;

using namespace std;
using namespace corsika::units::si;

//
// This example demonstrates the energy loss of muons as function of beta*gamma (=p/m)
//
int main() {
  feenableexcept(FE_INVALID);

  // setup environment, geometry
  using EnvType = Environment<setup::IEnvironmentModel>;
  EnvType env;

  const CoordinateSystem& rootCS = env.GetCoordinateSystem();

  process::energy_loss::EnergyLoss eLoss;

  setup::Stack stack;

  std::ofstream file("dEdX.dat");
  file << "# beta*gamma, dE/dX / eV/(g/cm²)" << std::endl;

  for (HEPEnergyType E0 = 300_MeV; E0 < 1_PeV; E0 *= 1.05) {
    stack.Clear();
    const Code beamCode = Code::MuPlus;
    const HEPMassType mass = GetMass(beamCode);
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
    auto plab = corsika::stack::MomentumVector(rootCS, {px, py, pz});
    cout << "input particle: " << beamCode << endl;
    cout << "input angles: theta=" << theta << " phi=" << phi << endl;
    cout << "input momentum: " << plab.GetComponents() / 1_GeV << endl;
    Point pos(rootCS, 0_m, 0_m,
              112.8_km); // this is the CORSIKA 7 start of atmosphere/universe
    stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            beamCode, E0, plab, pos, 0_ns});

    auto const p = stack.GetNextParticle();
    HEPEnergyType dE = eLoss.TotalEnergyLoss(p, 1_g / square(1_cm));
    file << P0 / mass << "\t" << -dE / 1_eV << std::endl;
  }
}
