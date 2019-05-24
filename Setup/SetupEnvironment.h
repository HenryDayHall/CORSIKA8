/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_setup_environment_h_
#define _include_corsika_setup_environment_h_

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NameModel.h>

namespace corsika::setup {
  using IEnvironmentModel = environment::IMediumModel;
  using SetupEnvironment = environment::Environment<IEnvironmentModel>;
} // namespace corsika::setup

#endif
