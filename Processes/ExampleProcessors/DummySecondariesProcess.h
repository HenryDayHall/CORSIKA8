/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/SecondariesProcess.h>

#include <chrono>
#include <thread>

namespace corsika::process {
  namespace example_processors {

    template <int ISleep>
    class DummySecondariesProcess : SecondariesProcess<DummySecondariesProcess<ISleep>> {
    private:
    public:
      template <typename TSecondaries>
      inline EProcessReturn DoSecondaries(TSecondaries&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return process::EProcessReturn::eOk;
      }
    };

  } // namespace example_processors
} // namespace corsika::process