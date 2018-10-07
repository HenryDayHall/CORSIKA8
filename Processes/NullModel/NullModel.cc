
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

using namespace corsika::process::null_model;

NullModel::NullModel() {}

NullModel::~NullModel() {}

void NullModel::init() {}

void NullModel::run() {}

double NullModel::GetStepLength() { return 0; }
