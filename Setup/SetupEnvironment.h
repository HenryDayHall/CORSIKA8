/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NameModel.h>

namespace corsika::setup {
  using IEnvironmentModel = environment::IMediumModel;
  using SetupEnvironment = environment::Environment<IEnvironmentModel>;
} // namespace corsika::setup
