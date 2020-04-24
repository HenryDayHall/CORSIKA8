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
#include <corsika/process/sibyll/ParticleConversion.h>

using namespace corsika::process::sibyll;

corsika::units::si::HEPMassType corsika::process::sibyll::GetSibyllMass(
    corsika::particles::Code const pCode) {
  using namespace corsika::units;
  using namespace corsika::units::si;
  if (pCode == corsika::particles::Code::Nucleus)
    throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
  auto sCode = ConvertToSibyllRaw(pCode);
  if (sCode == 0)
    return -1_GeV;
  else
    return sqrt(get_sibyll_mass2(sCode)) * 1_GeV;
}

// const std::map<sibyll::PID, ParticleProperties::InternalParticleCode>
//   process::sibyll::Sibyll2Corsika = {
//        {PID::E_MINUS, InternalParticleCode::Electron},
//};
