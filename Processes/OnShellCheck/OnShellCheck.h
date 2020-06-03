/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_on_shell_check_OnShellCheck_h_
#define _corsika_process_on_shell_check_OnShellCheck_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {
  namespace on_shell_check {
    class OnShellCheck : public process::SecondariesProcess<OnShellCheck> {

    public:
      OnShellCheck(const double vMassTolerance, const double vEnergyTolerance)
          : mass_tolerance_(vMassTolerance)
          , energy_tolerance_(vEnergyTolerance) {}

      EProcessReturn DoSecondaries(corsika::setup::StackView&);

      void Init();

    private:
      double mass_tolerance_;
      double energy_tolerance_;
    };
  } // namespace on_shell_check
} // namespace corsika::process

#endif
