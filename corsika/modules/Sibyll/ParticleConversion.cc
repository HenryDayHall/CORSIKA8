/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/process/sibyll/ParticleConversion.h>

using namespace corsika::sibyll;

corsika::units::si::HEPMassType corsika::process::sibyll::GetSibyllMass(
    corsika::particles::Code const pCode) {
  using namespace corsika::units;
  using namespace corsika::units::si;
  if (pCode == corsika::particles::Code::Nucleus)
    throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
  auto sCode = ConvertToSibyllRaw(pCode);
  if (sCode == 0)
    throw std::runtime_error("GetSibyllMass: unknown particle!");
  else
    return sqrt(get_sibyll_mass2(sCode)) * 1_GeV;
}
