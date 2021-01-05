/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/stack/SimpleStack.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace corsika;
using namespace std;

void fill(SimpleStack& s) {
  CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();
  for (int i = 0; i < 11; ++i) {
    s.addParticle(std::make_tuple(Code::Electron, 1.5_GeV * i,
                                  MomentumVector(rootCS, {0_GeV, 0_GeV, 1_GeV}),
                                  Point(rootCS, 0_m, 0_m, 0_m), 0_ns));
  }
}

void read(SimpleStack& s) {
  assert(s.getEntries() == 11); // stack has 11 particles

  HEPEnergyType total_energy;
  int i = 0;
  for (auto& p : s) {
    total_energy += p.getEnergy();
    // particles are electrons with 1.5 GeV energy times i
    assert(p.getPID() == Code::Electron);
    assert(p.getEnergy() == 1.5_GeV * (i++));
  }
}

int main() {

  std::cout << "stack_example" << std::endl;
  SimpleStack s;
  fill(s);
  read(s);
  return 0;
}
