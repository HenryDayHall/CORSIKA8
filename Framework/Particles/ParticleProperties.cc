
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/particles/ParticleProperties.h>
#include <iostream>

namespace corsika::particles {

  std::ostream& operator<<(std::ostream& stream, corsika::particles::Code const p) {
    return stream << corsika::particles::GetName(p);
  }
  
  Code ConvertFromPDG(PDGCode p) {
      static_assert(detail::conversionArray.size() % 2 == 1);
      auto constexpr maxPDG{(detail::conversionArray.size() - 1) >> 1};
      auto k = static_cast<PDGCodeType>(p);
      if (abs(k) <= maxPDG) {
          return detail::conversionArray[k + maxPDG];
      } else {
          return detail::conversionMap.at(p);
      }
  }

} // namespace corsika::particles
