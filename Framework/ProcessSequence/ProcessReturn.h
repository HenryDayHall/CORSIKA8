/*
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

  enum class EProcessReturn : int {
    eOk = (1 << 0),
    eParticleAbsorbed = (1 << 2),
    eInteracted = (1 << 3),
    eDecayed = (1 << 4),
  };

  inline EProcessReturn operator|(EProcessReturn a, EProcessReturn b) {
    return static_cast<EProcessReturn>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline EProcessReturn& operator|=(EProcessReturn& a, EProcessReturn b) {
    return a = a | b;
  }

  inline bool operator==(EProcessReturn a, EProcessReturn b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
  }

} // namespace corsika::process

#endif
