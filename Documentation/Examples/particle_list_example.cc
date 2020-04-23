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
#include <corsika/units/PhysicalUnits.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/process/sibyll/ParticleConversion.h>

#include <iomanip>
#include <iostream>
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace std;

//
// The example main program for a particle list
//
int main() {
  cout << "--------------------"
       << "particles in CORSIKA"
       << "--------------------" << endl;
  cout << std::setw(20) << "Name"
       << " | " << std::setw(10) << "PDG-id"
       << " |  " << std::setw(10) << "mass (GeV)"
       << " | " << std::setw(10) << "SIBYLL-id"
       << endl;
  cout << std::setw(58) << std::setfill('-') << "-" << endl;
  for (auto p : corsika::particles::detail::all_particles) {
    if (p != Code::Unknown && !IsNucleus(p))
      cout << std::setw(20) << std::setfill(' ') << p << " | "	
           << std::setw(10) << static_cast<int>(GetPDG(p)) << " | "
	   << std::setw(10) << GetMass(p) / 1_GeV << " | "
	   << std::setw(10) << static_cast<int>(corsika::process::sibyll::ConvertToSibyll(p)) << " | "
	   
	   << endl;
  }
  cout << std::setw(54) << std::setfill('-') << "-" << endl;
}
