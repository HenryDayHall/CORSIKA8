/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>

corsika::HEPMassType corsika::sibyll::GetSibyllMass(corsika::Code const pCode) {
  if (pCode == corsika::Code::Nucleus)
    throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
  auto sCode = ConvertToSibyllRaw(pCode);
  if (sCode == 0)
    throw std::runtime_error("GetSibyllMass: unknown particle!");
  else
    return sqrt(get_sibyll_mass2(sCode)) * 1_GeV;
}
