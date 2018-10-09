
/**
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

#include <map>

namespace corsika::process::sibyll {
  enum class Code : int8_t;
  using PIDIntType = std::underlying_type<Code>::type;

#include <corsika/processes/sibyll/Generated.inc>

  bool handledBySibyll(corsika::particles::Code pCode) {
    return handleable[static_cast<CodeIntType>(pCode)];
  }

  Code constexpr ConvertToSibyll(corsika::particles::Code pCode) {
    //~ assert(handledBySibyll(pCode));
    return static_cast<Code>(corsika2sibyll[static_cast<CodeIntType>(pCode)]);
  }

  corsika::particles::Code constexpr ConvertFromSibyll(Code pCode) {
    return sibyll2corsika[static_cast<PIDIntType>(pCode) - minSibyll];
  }

} // namespace corsika::process::sibyll

#endif
