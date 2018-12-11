
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_ProcessReturn_h_
#define _include_ProcessReturn_h_

namespace corsika::process {

  /**
     since in a process sequence many status updates can accumulate
     for a single particle, this enum should define only bit-flags
     that can be accumulated easily with "|="
   */

  enum class EProcessReturn {
    eOk = 1,
    eParticleAbsorbed = 2,
    eInteracted = 3,
    eDecayed = 4,
  };
} // namespace corsika::process

#endif
