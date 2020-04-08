/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <cstdlib>
#include <iostream>

using namespace corsika;
using namespace corsika::units::si;

int main() {
  random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");
  corsika::process::UrQMD::UrQMD urqmd;

  for (auto Elab = 100_MeV; Elab <= 10_TeV; Elab *= 1.02) {
    std::cout << Elab / 1_GeV << '\t'
              << urqmd.GetCrossSection(particles::Code::Proton, particles::Code::Proton,
                                       Elab) /
                     1_mb
              << std::endl;
  }

  return EXIT_SUCCESS;
}
