/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMagneticFieldModel.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/IMediumTypeModel.h>
#include <corsika/environment/IRefractiveIndexModel.h>
#include <corsika/environment/InhomogeneousMedium.h>

namespace corsika::setup {

  /**
     Definition of the default environemnt model interface. Each model
     interface provides properties of the environment in a position
     bdependent way. 
   */
  
  using IEnvironment = environment::IMediumTypeModel<environment::IMagneticFieldModel<environment::IMediumModel>>;
  using Environment = environment::Environment<IEnvironment>;
} // namespace corsika::setup
