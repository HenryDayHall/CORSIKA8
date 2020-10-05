/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BoundaryCrossingProcess.h>

#include <chrono>
#include <thread>

namespace corsika::process {
  namespace example_processors {

    template <int ISleep>
    class DummyBoundaryCrossingProcess
        : public BoundaryCrossingProcess<DummyBoundaryCrossingProcess<ISleep>> {
    private:
    public:
      template <typename Particle, typename VTNType>
      EProcessReturn DoBoundaryCrossing(Particle&, VTNType const&, VTNType const&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return EProcessReturn::eOk;
      }
    };

  } // namespace example_processors
} // namespace corsika::process