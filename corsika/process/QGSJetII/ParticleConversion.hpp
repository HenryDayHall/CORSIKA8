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

  /**
     These are the possible secondaries produced by QGSJetII
   */
  enum class QgsjetIICode : int8_t;
  using QgsjetIICodeIntType = std::underlying_type<QgsjetIICode>::type;

  /**
     These are the possible projectile for which QGSJetII knwos cross section
   */
  enum class QgsjetIIXSClass : int8_t {
    CannotInteract = 0,
    LightMesons = 1,
    Baryons = 2,
    Kaons = 3,
  };
  using QgsjetIIXSClassIntType = std::underlying_type<QgsjetIIXSClass>::type;

  /**
     These are the only possible projectile types in QGSJetII
   */
  enum class QgsjetIIHadronType : int8_t {
    UndefinedType = 0,
    PiPlusType = +1,
    PiMinusType = -1,
    ProtonType = +2,
    AntiProtonType = -2,
    NeutronType = +3,
    AntiNeutronType = -3,
    KaonPlusType = +4,
    KaonMinusType = -4,
    Kaon0LType = +5,
    Kaon0SType = -5,
    // special codes, not in QGSJetII
    NucleusType = 100,
    NeutralLightMesonType = 101,
  };
  using QgsjetIIHadronTypeIntType = std::underlying_type<QgsjetIIHadronType>::type;

#include <corsika/process/qgsjetII/Generated.inc>

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

} // namespace corsika::process::qgsjetII
