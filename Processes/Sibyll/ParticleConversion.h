
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_processes_sibyll_particles_h_
#define _include_processes_sibyll_particles_h_

#include <corsika/particles/ParticleProperties.h>

#include <bitset2/bitset2.hpp>

#include <map>

namespace corsika::process::sibyll {

  enum class SibyllCode : int8_t;
  using SibyllCodeIntType = std::underlying_type<SibyllCode>::type;

#include <corsika/process/sibyll/Generated.inc>

  bool constexpr KnownBySibyll(corsika::particles::Code pCode) {
    return isKnown[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

  bool constexpr CanInteract(corsika::particles::Code pCode) {
    return canInteract[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

  SibyllCode constexpr ConvertToSibyll(corsika::particles::Code pCode) {
    return static_cast<SibyllCode>(
        corsika2sibyll[static_cast<corsika::particles::CodeIntType>(pCode)]);
  }

  corsika::particles::Code constexpr ConvertFromSibyll(SibyllCode pCode) {
    return sibyll2corsika[static_cast<SibyllCodeIntType>(pCode) - minSibyll];
  }

  int constexpr ConvertToSibyllRaw(corsika::particles::Code pCode) {
    return static_cast<int>(ConvertToSibyll(pCode));
  }

  int constexpr GetSibyllXSCode(corsika::particles::Code pCode) {
    return corsika2sibyllXStype[static_cast<corsika::particles::CodeIntType>(pCode)];
  }

} // namespace corsika::process::sibyll

#endif
