/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>

#include <string>

namespace corsika::qgsjetII {

  enum class QgsjetIICode : int8_t;
  using QgsjetIICodeIntType = std::underlying_type<QgsjetIICode>::type;

#include <corsika/modules/qgsjetII/Generated.inc>

  QgsjetIICode constexpr ConvertToQgsjetII(corsika::Code pCode) {
    return static_cast<QgsjetIICode>(
        corsika2qgsjetII[static_cast<corsika::CodeIntType>(pCode)]);
  }

  corsika::Code constexpr ConvertFromQgsjetII(QgsjetIICode pCode) {
    auto const pCodeInt = static_cast<QgsjetIICodeIntType>(pCode);
    auto const corsikaCode = qgsjetII2corsika[pCodeInt - minQgsjetII];
    if (corsikaCode == corsika::Code::Unknown) {
      throw std::runtime_error(std::string("QGSJETII/CORSIKA conversion of pCodeInt=")
                                   .append(std::to_string(pCodeInt))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr ConvertToQgsjetIIRaw(corsika::Code pCode) {
    return static_cast<int>(ConvertToQgsjetII(pCode));
  }

  int constexpr GetQgsjetIIXSCode(corsika::Code pCode) {
    if (pCode == corsika::Code::Nucleus) return 2;
    return corsika2qgsjetIIXStype[static_cast<corsika::CodeIntType>(pCode)];
  }

  bool constexpr CanInteract(corsika::Code pCode) {
    return (GetQgsjetIIXSCode(pCode) > 0) && (ConvertToQgsjetIIRaw(pCode) <= 5);
  }

} // namespace corsika::qgsjetII

#include <corsika/detail/modules/qgsjetII/ParticleConversion.inl>
