/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <epos.hpp>

namespace corsika::epos {

  inline HEPMassType getEposMass(Code const pCode) {
    if (is_nucleus(pCode)) throw std::runtime_error("Not suited for Nuclei.");
    auto sCode = convertToEposRaw(pCode);
    if (sCode == 0)
      throw std::runtime_error("getEposMass: unknown particle!");
    else {
      float mass2 = 0;
      ::epos::idmass_(sCode, mass2);
      return sqrt(mass2) * 1_GeV;
    }
  }

  inline PDGCode getEposPDGId(Code const p) {
    if (!is_nucleus(p)) {
      int eid = corsika::epos::convertToEposRaw(p);
      char nxs[4] = "nxs";
      char pdg[4] = "pdg";
      return static_cast<PDGCode>(::epos::idtrafo_(nxs, pdg, eid));
    } else {
      throw std::runtime_error("Epos id conversion not implemented for nuclei!");
    }
  }
} // namespace corsika::epos
