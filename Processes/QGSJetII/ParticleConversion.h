/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>

#include <string>

namespace corsika::process::qgsjetII {

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

  QgsjetIICode constexpr ConvertToQgsjetII(corsika::particles::Code pCode) {
    return corsika2qgsjetII[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

  corsika::particles::Code constexpr ConvertFromQgsjetII(QgsjetIICode pCode) {
    auto const pCodeInt = static_cast<QgsjetIICodeIntType>(pCode);
    auto const corsikaCode = qgsjetII2corsika[pCodeInt - minQgsjetII];
    if (corsikaCode == corsika::particles::Code::Unknown) {
      throw std::runtime_error(std::string("QGSJETII/CORSIKA conversion of pCodeInt=")
                                   .append(std::to_string(pCodeInt))
                                   .append(" impossible"));
    }
    return corsikaCode;
  }

  QgsjetIICodeIntType constexpr ConvertToQgsjetIIRaw(corsika::particles::Code pCode) {
    return static_cast<QgsjetIICodeIntType>(ConvertToQgsjetII(pCode));
  }

  QgsjetIIXSClass constexpr GetQgsjetIIXSCode(corsika::particles::Code pCode) {
    // if (pCode == corsika::particles::Code::Nucleus)
    // static_cast(QgsjetIIXSClassIntType>();
    return corsika2qgsjetIIXStype[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

  QgsjetIIXSClassIntType constexpr GetQgsjetIIXSCodeRaw(corsika::particles::Code pCode) {
    return static_cast<QgsjetIIXSClassIntType>(GetQgsjetIIXSCode(pCode));
  }

  bool constexpr CanInteract(corsika::particles::Code pCode) {
    return GetQgsjetIIXSCode(pCode) != QgsjetIIXSClass::CannotInteract;
  }

  QgsjetIIHadronType constexpr GetQgsjetIIHadronType(corsika::particles::Code pCode) {
    return corsika2qgsjetIIHadronType[static_cast<corsika::particles::CodeIntType>(
        pCode)];
  }

} // namespace corsika::process::qgsjetII
