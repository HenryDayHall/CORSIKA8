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

  enum class EposCode : int16_t;
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

  EposCode constexpr convertToEpos(corsika::Code pCode) {
    return corsika2epos[static_cast<corsika::CodeIntType>(pCode)];
  }

  corsika::Code constexpr convertFromEpos(EposCode pCode) {
    auto const s = static_cast<EposCodeIntType>(pCode);
    auto const corsikaCode = epos2corsika[s - minEpos];
    if (corsikaCode == corsika::Code::Unknown) {
      throw std::runtime_error(std::string("EPOS/CORSIKA conversion of ")
                                   .append(std::to_string(s))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr convertToEposRaw(corsika::Code pCode) {
    return static_cast<int>(convertToEpos(pCode));
  }

  int constexpr getEposXSCode(corsika::Code pCode) {
    return static_cast<EposXSClassIntType>(
        corsika2eposXStype[static_cast<corsika::CodeIntType>(pCode)]);
  }

  bool constexpr canInteract(corsika::Code pCode) { return getEposXSCode(pCode) > 0; }

  HEPMassType getEposMass(corsika::Code const);

  PDGCode getEposPDGId(corsika::Code const);

} // namespace corsika::epos

#include <corsika/detail/modules/epos/ParticleConversion.inl>
