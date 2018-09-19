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
  };
} // namespace corsika::process

#endif
