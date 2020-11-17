/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/stack/SuperStupidStack.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace corsika;
using namespace corsika::units::si;
using namespace std;

void fill(corsika::super_stupid::SuperStupidStack& s) {
  const corsika::CoordinateSystem& rootCS =
      corsika::RootCoordinateSystem::getInstance().GetRootCoordinateSystem();
  for (int i = 0; i < 11; ++i) {
    s.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{
            corsika::Code::Electron, 1.5_GeV * i,
            corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 1_GeV}),
            corsika::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
  }
}

void read(corsika::super_stupid::SuperStupidStack& s) {
  assert(s.GetSize() == 11); // stack has 11 particles

  HEPEnergyType total_energy;
  for (const auto& p : s) {
    total_energy += p.GetEnergy();
    // particles are electrons with 1.5 GeV energy times i
    assert(p.GetPID() == corsika::Code::Electron);
    assert(p.GetEnergy() == 1.5_GeV * (i++));
  }
}

int main() {
  corsika::super_stupid::SuperStupidStack s;
  fill(s);
  read(s);
  return 0;
}
