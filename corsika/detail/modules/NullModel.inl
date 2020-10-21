/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/NullModel.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika::null_model {

  void NullModel::Init() {}

  NullModel::NullModel(LengthType maxStepLength)
      : fMaxStepLength(maxStepLength) {}

  template <>
  corsika::EProcessReturn NullModel::DoContinuous(corsika::setup::Stack::ParticleType&,
                                                  corsika::setup::Trajectory&) const {
    return corsika::EProcessReturn::eOk;
  }

  template <>
  LengthType NullModel::MaxStepLength(corsika::setup::Stack::ParticleType&,
                                      corsika::setup::Trajectory&) const {
    return fMaxStepLength;
  }

} // namespace corsika::null_model
