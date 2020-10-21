/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/BaseProcess.hpp>

namespace corsika::null_model {

  class NullModel : public corsika::BaseProcess<NullModel> {
    LengthType const fMaxStepLength;

  public:
    NullModel(LengthType maxStepLength = meter * std::numeric_limits<double>::infinity());

    void Init();

    template <typename Particle, typename Track>
    corsika::EProcessReturn DoContinuous(Particle&, Track&) const;

    template <typename Particle, typename Track>
    LengthType MaxStepLength(Particle&, Track&) const;
  };

} // namespace corsika::null_model

#include <corsika/detail/modules/NullModel.inl>
