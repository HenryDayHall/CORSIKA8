
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/null_model/NullModel.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using namespace corsika;
using namespace corsika::process::null_model;

void corsika::process::null_model::NullModel::Init() {}

NullModel::NullModel(units::si::LengthType maxStepLength)
    : fMaxStepLength(maxStepLength) {}

template <>
process::EProcessReturn NullModel::DoContinuous(setup::Stack::ParticleType&,
                                                setup::Trajectory&, setup::Stack&) const {
  return process::EProcessReturn::eOk;
}

template <>
units::si::LengthType NullModel::MaxStepLength(setup::Stack::ParticleType&,
                                               setup::Trajectory&) const {
  return fMaxStepLength;
}
