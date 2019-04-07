
/*
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

#include <corsika/process/StackProcess.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

  namespace stack_inspector {

    template <typename Stack>
    class StackInspector : public corsika::process::StackProcess<StackInspector<Stack>> {

      typedef typename Stack::ParticleType Particle;

    public:
      StackInspector(const bool aReport);
      ~StackInspector();

      void Init();
      EProcessReturn DoStack(Stack&);

    private:
      bool fReport;
      int fCountStep = 0;
    };

  } // namespace stack_inspector

} // namespace corsika::process

#endif
