
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/null_model/NullModel.h>


#include <corsika/logging/Logger.h>
#include <corsika/setup/SetupTrajectory.h>

#include <iostream>
#include <limits>
using namespace std;

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process::null_model;

template <typename Stack>
NullModel<Stack>::NullModel() {}

template <typename Stack>
NullModel<Stack>::~NullModel() {}

template <typename Stack>
process::EProcessReturn NullModel<Stack>::DoContinuous(Particle&, setup::Trajectory&,
						       Stack& ) const {
  return EProcessReturn::eOk;
}

template <typename Stack>
double NullModel<Stack>::MaxStepLength(Particle&, setup::Trajectory&) const {
  return std::numeric_limits<double>::infinity();
}

template <typename Stack>
void NullModel<Stack>::Init() {
}

#include <corsika/setup/SetupStack.h>

template class process::null_model::NullModel<setup::Stack>;

