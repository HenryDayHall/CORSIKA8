/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IMediumPropertyModel.hpp>
#include <corsika/media/IRefractiveIndexModel.hpp>

namespace corsika::setup {

  /**
     Definition of the default environemnt model interface. Each model
     interface provides properties of the environment in a position
     bdependent way.
   */

  using EnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
  using Environment = Environment<EnvironmentInterface>;

} // end namespace corsika::setup
