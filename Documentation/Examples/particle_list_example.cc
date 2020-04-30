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
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/units/PhysicalUnits.h>

#include <iomanip>
#include <iostream>
#include <string>
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace std;

//
// The example main program for a particle list
//
int main() {
  cout << "------------------------------------------"
       << "particles in CORSIKA"
       << "------------------------------------------" << endl;
  cout << std::setw(20) << "Name"
       << " | " << std::setw(10) << "PDG-id"
       << " | " << std::setw(10) << "SIBYLL-id"
       << " | " << std::setw(10) << "QGSJETII-id"
       << " | " << std::setw(18) << "PDG-mass (GeV)"
       << " | " << std::setw(18) << "SIBYLL-mass (GeV)"
       << " | " << endl;
  cout << std::setw(104) << std::setfill('-') << "-" << endl;
  for (auto p : getAllParticles()) {
    if (!IsNucleus(p)) {
      auto const sib_id = static_cast<int>(corsika::process::sibyll::ConvertToSibyll(p));
      auto const sib_mass = corsika::process::sibyll::GetSibyllMass(p) / 1_GeV;
      auto const qgs_id =
          static_cast<int>(corsika::process::qgsjetII::ConvertToQgsjetII(p));
      cout << std::setw(20) << std::setfill(' ') << p << " | " << std::setw(10)
           << static_cast<int>(GetPDG(p)) << " | " << std::setw(10)
           << (sib_id != 0 ? to_string(sib_id) : "--") << " | " << std::setw(10)
           << (qgs_id != 0 ? to_string(qgs_id) : "--") << " | " << std::setw(18)
           << std::setprecision(5) << GetMass(p) / 1_GeV << " | " << std::setw(18)
           << std::setprecision(5) << (sib_mass != -1 ? to_string(sib_mass) : "--")
           << " | " << endl;
    }
  }
  cout << std::setw(104) << std::setfill('-') << "-" << endl;
}
