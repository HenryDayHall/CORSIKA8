/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_particle_cut_CONEXSourceCut_h_
#define _corsika_process_particle_cut_CONEXSourceCut_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {
  namespace conex_source_cut {
    class CONEXSourceCut : public process::SecondariesProcess<CONEXSourceCut> {

    public:
      CONEXSourceCut();
      corsika::process::EProcessReturn DoSecondaries(corsika::setup::StackView&);

      void Init();

    private:
      static std::array<particles::Code, 3> constexpr em_codes_{
          particles::Code::Gamma, particles::Code::Electron, particles::Code::Positron};
    };
  } // namespace conex_source_cut
} // namespace corsika::process

#endif
