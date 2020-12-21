/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/modules/QGSJetII.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <iomanip>
#include <string>

using namespace corsika;
using namespace std;

//
// The example main program for a particle list
//
int main() {

  logging::set_level(spdlog::level::info);

  logging::info(
      "------------------------------------------\n"
      "particles in CORSIKA\n"
      "------------------------------------------\n"
      "Name                | "
      "PDG-id     | "
      "SIBYLL-id  | "
      "QGSJETII-id| "
      "PDG-mass (GeV)   | "
      "SIBYLL-mass (GeV)|\n"
      "{:-}",
      "", 104);
  for (auto p : get_all_particles()) {
    if (!is_nucleus(p)) {
      corsika::sibyll::SibyllCode sib_id = corsika::sibyll::convertToSibyll(p);
      auto const sib_mass = (sib_id != corsika::sibyll::SibyllCode::Unknown
                                 ? to_string(corsika::sibyll::getSibyllMass(p) / 1_GeV)
                                 : "--");
      auto const qgs_id = corsika::qgsjetII::convertToQgsjetII(p);
      logging::info("{:20} | {:10} | {:10} | {:10.5} | {:18.5}", p,
                    static_cast<int>(get_PDG(p)),
                    (sib_id != corsika::sibyll::SibyllCode::Unknown
                         ? to_string(static_cast<int>(sib_id))
                         : "--"),
                    (qgs_id != corsika::qgsjetII::QgsjetIICode::Unknown
                         ? to_string(static_cast<int>(qgs_id))
                         : "--"),
                    get_mass(p) / 1_GeV, sib_mass);
    }
  }
  logging::info("{:-104}", "");
}
