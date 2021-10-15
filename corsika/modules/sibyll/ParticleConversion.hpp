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

#include <sibyll2.3d.hpp>

#include <string>

namespace corsika::sibyll {

  enum class SibyllCode : int8_t;
  using SibyllCodeIntType = std::underlying_type<SibyllCode>::type;

  /**
     These are the possible projectile for which Sibyll knows the cross section
   */
  enum class SibyllXSClass : int8_t {
    CannotInteract = 0,
    Baryon = 1,
    Pion = 2,
    Kaon = 3,
  };
  using SibyllXSClassIntType = std::underlying_type<SibyllXSClass>::type;

#include <corsika/modules/sibyll/Generated.inc>

  SibyllCode constexpr convertToSibyll(Code const pCode) {
    return corsika2sibyll[static_cast<CodeIntType>(pCode)];
  }

  Code constexpr convertFromSibyll(SibyllCode const pCode) {
    auto const s = static_cast<SibyllCodeIntType>(pCode);
    auto const corsikaCode = sibyll2corsika[s - minSibyll];
    if (corsikaCode == Code::Unknown) {
      throw std::runtime_error(std::string("SIBYLL/CORSIKA conversion of ")
                                   .append(std::to_string(s))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr convertToSibyllRaw(Code const code) {
    return static_cast<int>(convertToSibyll(code));
  }

  int constexpr getSibyllXSCode(Code const code) {
    if (is_nucleus(code))
      return static_cast<SibyllXSClassIntType>(SibyllXSClass::CannotInteract);
    return static_cast<SibyllXSClassIntType>(
        corsika2sibyllXStype[static_cast<CodeIntType>(code)]);
  }

  bool constexpr canInteract(Code const pCode) { return getSibyllXSCode(pCode) > 0; }

  HEPMassType getSibyllMass(Code const);

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/ParticleConversion.inl>
