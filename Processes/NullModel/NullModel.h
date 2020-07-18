/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::null_model {

  class NullModel : public corsika::process::BaseProcess<NullModel> {
    corsika::units::si::LengthType const fMaxStepLength;

  public:
    NullModel(corsika::units::si::LengthType maxStepLength =
                  corsika::units::si::meter * std::numeric_limits<double>::infinity());

    template <typename Particle, typename Track>
    process::EProcessReturn DoContinuous(Particle&, Track&) const;

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle&, Track&) const;
  };

} // namespace corsika::process::null_model
