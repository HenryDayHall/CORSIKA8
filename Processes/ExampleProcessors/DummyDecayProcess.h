/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/DecayProcess.h>
#include <corsika/units/PhysicalUnits.h>

#include <chrono>
#include <thread>

namespace corsika::process {
  namespace example_processors {

    template <int ISleep>
    class DummyDecayProcess : DecayProcess<DummyDecayProcess<ISleep>> {
    private:
    public:
      template <typename Particle>
      EProcessReturn DoDecay(Particle&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return process::EProcessReturn::eOk;
      }

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle&) {
        using namespace corsika::units::si;

        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return std::numeric_limits<double>::infinity() * 1_s;
      }
    };

  } // namespace example_processors
} // namespace corsika::process