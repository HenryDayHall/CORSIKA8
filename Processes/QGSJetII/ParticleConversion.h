/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_processes_qgsjetII_particles_h_
#define _include_processes_qgsjetII_particles_h_

#include <corsika/particles/ParticleProperties.h>

#include <string>

namespace corsika::process::qgsjetII {

  enum class QgsjetIICode : int8_t;
  using QgsjetIICodeIntType = std::underlying_type<QgsjetIICode>::type;

#include <corsika/process/qgsjetII/Generated.inc>

  QgsjetIICode constexpr ConvertToQgsjetII(corsika::particles::Code pCode) {
    return static_cast<QgsjetIICode>(
        corsika2qgsjetII[static_cast<corsika::particles::CodeIntType>(pCode)]);
  }

  corsika::particles::Code constexpr ConvertFromQgsjetII(QgsjetIICode pCode) {
    auto const s = static_cast<QgsjetIICodeIntType>(pCode);
    auto const corsikaCode = qgsjetII2corsika[s - minQgsjetII];
    if (corsikaCode == corsika::particles::Code::Unknown) {
      throw std::runtime_error(std::string("QGSJETII/CORSIKA conversion of ")
                                   .append(std::to_string(s))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  int constexpr ConvertToQgsjetIIRaw(corsika::particles::Code pCode) {
    return static_cast<int>(ConvertToQgsjetII(pCode));
  }

  int constexpr GetQgsjetIIXSCode(corsika::particles::Code pCode) {
    if (pCode == corsika::particles::Code::Nucleus) return 2;
    return corsika2qgsjetIIXStype[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

  bool constexpr CanInteract(corsika::particles::Code pCode) {
    return (GetQgsjetIIXSCode(pCode) > 0) && (ConvertToQgsjetIIRaw(pCode) <= 5);
  }

} // namespace corsika::process::qgsjetII

#endif
