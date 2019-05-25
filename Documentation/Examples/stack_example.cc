/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::stack;
using namespace corsika::geometry;
using namespace std;

void fill(corsika::stack::super_stupid::SuperStupidStack& s) {
  const geometry::CoordinateSystem& rootCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  for (int i = 0; i < 11; ++i) {
    s.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV * i,
            corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 1_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
  }
}

void read(corsika::stack::super_stupid::SuperStupidStack& s) {
  assert(s.GetSize() == 11); // stack has 11 particles

  HEPEnergyType total_energy;
  int i = 0;
  for (auto& p : s) {
    total_energy += p.GetEnergy();
    // particles are electrons with 1.5 GeV energy times i
    assert(p.GetPID() == particles::Code::Electron);
    assert(p.GetEnergy() == 1.5_GeV * (i++));
  }
}

int main() {
  corsika::stack::super_stupid::SuperStupidStack s;
  fill(s);
  read(s);
  return 0;
}
