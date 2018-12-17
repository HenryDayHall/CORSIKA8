
/**
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
#include <corsika/setup/SetupTrajectory.h>

namespace corsika::process {

  namespace null_model {

    template <typename Stack>
    class NullModel {

      typedef typename Stack::ParticleType Particle;

    public:
      NullModel();
      ~NullModel();

      void Init();
      EProcessReturn DoContinuous(Particle&, corsika::setup::Trajectory&, Stack& s) const;
      double MaxStepLength(Particle&, corsika::setup::Trajectory&) const;
    };

  } // namespace null_model

} // namespace corsika::process

#endif
