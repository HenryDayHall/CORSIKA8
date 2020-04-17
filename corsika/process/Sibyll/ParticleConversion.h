/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>

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

#include <corsika/process/sibyll/Generated.inc>

  SibyllCode constexpr ConvertToSibyll(corsika::Code pCode) {
    return static_cast<SibyllCode>(
        corsika2sibyll[static_cast<corsika::CodeIntType>(pCode)]);
  }

  corsika::Code constexpr ConvertFromSibyll(SibyllCode pCode) {
    auto const s = static_cast<SibyllCodeIntType>(pCode);
    auto const corsikaCode = sibyll2corsika[s - minSibyll];
    if (corsikaCode == corsika::Code::Unknown) {
      throw std::runtime_error(std::string("SIBYLL/CORSIKA conversion of ")
                                   .append(std::to_string(s))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr ConvertToSibyllRaw(corsika::Code pCode) {
    return static_cast<int>(ConvertToSibyll(pCode));
  }

  int constexpr GetSibyllXSCode(corsika::Code pCode) {
    return corsika2sibyllXStype[static_cast<corsika::CodeIntType>(pCode)];
  }

  bool constexpr CanInteract(corsika::Code pCode) {
    return GetSibyllXSCode(pCode) > 0;
  }

} // namespace corsika::sibyll

} // namespace corsika::process::sibyll
