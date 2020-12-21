/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/BaseProcess.hpp>

namespace corsika {

  /**
   * Process that does nothing
   */

  class NullModel : public BaseProcess<NullModel> {

  public:
    NullModel() = default;
    ~NullModel() = default;
  };

} // namespace corsika
