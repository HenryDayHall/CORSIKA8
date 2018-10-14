
/**
  * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
  * 
  * See file AUTHORS for a list of contributors.
  * 
  * This software is distributed under the terms of the GNU General Public
  * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
  * the license.
*/

#ifndef _Physics_StackInspector_StackInspector_h_
#define _Physics_StackInspector_StackInspector_h_

#include <corsika/process/BaseProcess.h>

namespace corsika::process {

  namespace stack_inspector {

    template <typename Stack, typename Trajectory>
    class StackInspector
        : public corsika::process::BaseProcess<StackInspector<Stack, Trajectory>> {

      typedef typename Stack::ParticleType Particle;

    public:
      StackInspector(const bool aReport);
      ~StackInspector();

      void Init();

      // template <typename Particle, typename Trajectory, typename Stack>
      EProcessReturn DoContinuous(Particle&, Trajectory&, Stack& s) const;

      // template <typename Particle>
      double MinStepLength(Particle&) const;

      // template <typename Particle, typename Stack>
      void DoDiscrete(Particle&, Stack&) const;

    private:
      bool fReport;
    };

  } // namespace stack_inspector

} // namespace corsika::process

#endif
