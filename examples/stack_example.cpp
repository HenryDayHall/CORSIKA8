/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/stack/SuperStupidStack.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace corsika;
using namespace std;

void fill(simple_stack::SuperStupidStack& s) {
  const CoordinateSystemPtr& rootCS = get_root_CoordinateSystem();
  for (int i = 0; i < 11; ++i) {
    s.addParticle(
        std::tuple<Code, HEPEnergyType, simple_stack::MomentumVector, Point,
                   TimeType>{Code::Electron, 1.5_GeV * i,
                             simple_stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 1_GeV}),
                             Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
  }
}

void read(simple_stack::SuperStupidStack& s) {
  assert(s.getSize() == 11); // stack has 11 particles

  HEPEnergyType total_energy;
  [[maybe_unused]] int i = 0;
  for (const auto& p : s) {
    total_energy += p.getEnergy();
    // particles are electrons with 1.5 GeV energy times i
    assert(p.getPID() == Code::Electron);
    assert(p.getEnergy() == 1.5_GeV * (i++));
  }
}

int main() {
  simple_stack::SuperStupidStack s;
  fill(s);
  read(s);
  return 0;
}
