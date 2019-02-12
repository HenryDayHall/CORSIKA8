
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Physics_NullModel_NullModel_h_
#define _Physics_NullModel_NullModel_h_

#include <corsika/process/ContinuousProcess.h>

namespace corsika::process::null_model {

  class NullModel : public corsika::process::ContinuousProcess<NullModel> {
    corsika::units::si::LengthType const fMaxStepLength;

  public:
    NullModel(corsika::units::si::LengthType maxStepLength =
                  corsika::units::si::meter * std::numeric_limits<double>::infinity());

    void Init();

    template <typename Particle, typename Track, typename Stack>
    process::EProcessReturn DoContinuous(Particle&, Track&, Stack&) const;

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle&, Track&) const;
  };

} // namespace corsika::process::null_model

#endif
