/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NameModel.hpp>

namespace corsika::setup {
  using IEnvironmentModel = corsika::IMediumModel;
  using SetupEnvironment = corsika::Environment<IEnvironmentModel>;
} // namespace corsika::setup
