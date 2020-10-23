/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>

namespace corsika::process {

  class NullModel : public corsika::process::BaseProcess<NullModel> {

  public:
    NullModel() = default;
    ~NullModel() = default;
  };

} // namespace corsika::process
