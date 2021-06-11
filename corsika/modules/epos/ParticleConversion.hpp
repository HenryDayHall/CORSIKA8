/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <epos.hpp>

#include <string>

namespace corsika::epos {

  enum class EposCode : int32_t;
  using EposCodeIntType = std::underlying_type<EposCode>::type;

  /**
     These are the possible projectile for which Epos knows the cross section
   */
  enum class EposXSClass : int8_t {
    CannotInteract = 0,
    Baryon = 2,
    Pion = 1,
    Kaon = 3,
    Charm = 4,
  };
  using EposXSClassIntType = std::underlying_type<EposXSClass>::type;

#include <corsika/modules/epos/Generated.inc>

  EposCode constexpr convertToEpos(Code pCode) {
    return corsika2epos[static_cast<CodeIntType>(pCode)];
  }

  Code constexpr convertFromEpos(EposCode pCode) {
    EposCodeIntType const s = static_cast<EposCodeIntType>(pCode);
    // if nucleus (pdg-id)
    if (s >= 1000000000) { return Code::Nucleus; }
    auto const corsikaCode = epos2corsika[s - minEpos];
    if (corsikaCode == Code::Unknown) {
      throw std::runtime_error(std::string("EPOS/CORSIKA conversion of ")
                                   .append(std::to_string(s))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr convertToEposRaw(Code pCode) {
    return static_cast<int>(convertToEpos(pCode));
  }

  int constexpr getEposXSCode(Code pCode) {
    return static_cast<EposXSClassIntType>(
        corsika2eposXStype[static_cast<CodeIntType>(pCode)]);
  }

  bool constexpr canInteract(Code pCode) { return getEposXSCode(pCode) > 0; }

  HEPMassType getEposMass(Code const);

  PDGCode getEposPDGId(Code const);

} // namespace corsika::epos

#include <corsika/detail/modules/epos/ParticleConversion.inl>
