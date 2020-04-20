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
#include <fstream>
#include <iostream>

using namespace corsika;
using namespace corsika::units::si;

int main() {
  random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");
  corsika::process::UrQMD::UrQMD urqmd;

  std::vector<particles::Code> const projectiles{
      {particles::Code::Proton, particles::Code::AntiProton, particles::Code::Neutron,
       particles::Code::AntiNeutron, particles::Code::PiPlus, particles::Code::PiMinus,
       particles::Code::KPlus, particles::Code::KMinus, particles::Code::K0Short}};

  for (auto const& p : projectiles) {
    std::ofstream file(std::string("xs_") + particles::GetName(p) + ".dat");
    for (auto Elab = particles::GetMass(p) + 200_MeV; Elab <= 10_TeV; Elab *= 1.02) {
      file << Elab / 1_GeV << '\t'
           << urqmd.GetTabulatedCrossSection(p, particles::Code::Nitrogen, Elab) / 1_mb
           << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
