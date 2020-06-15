/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/conex_source_cut/CONEXSourceCut.h>
#include <algorithm>
#include <iomanip>

using namespace std;

using namespace corsika::process::conex_source_cut;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace corsika::setup;

corsika::process::EProcessReturn CONEXSourceCut::DoSecondaries(
    corsika::setup::StackView& vS) {
  auto p = vS.begin();
  while (p != vS.end()) {
    Code const pid = p.GetPID();
    HEPEnergyType const energy = p.GetEnergy();

    if (std::find(em_codes_.cbegin(), em_codes_.cend(), pid) != em_codes_.cend()) {
      std::cout << "CONEXSourceCut: removing " << pid << " " << std::scientific << energy
                << std::endl;

      p.Delete();
    }

    else {
      ++p;
    }
  }

  return corsika::process::EProcessReturn::eOk;
}

void CONEXSourceCut::Init() {}

CONEXSourceCut::CONEXSourceCut() {}
