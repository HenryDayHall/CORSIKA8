/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika::environment {

  /**
   * Medium types are useful most importantly for effective models
   * like energy losses. a particular medium (mixture of components)
   * may have specif properties not reflected by its mixture of
   * components. 
   */
  
  enum class EMediumType { eUnknown, eAir, eWater, eIce, eRock };

}
