/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <chrono>
#include <thread>

#include <corsika/process/SecondariesProcess.h>

namespace corsika::process {
  namespace devtools {

    template <int ISleep>
    class DummySecondariesProcess : SecondariesProcess<DummySecondariesProcess<ISleep>> {
    private:
    public:
      template <typename TSecondaries>
      inline EProcessReturn DoSecondaries(TSecondaries&)
      {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(ISleep));
        return process::EProcessReturn::eOk;
      }
    };

  } // namespace devtools
} // namespace corsika::process