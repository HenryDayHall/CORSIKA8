/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/ContinuousProcess.h>

#include <chrono>
#include <thread>

namespace corsika::process {
  namespace devtools {

    template <int ISleep>
    class DummyContinuousProcess : public ContinuousProcess<DummyContinuousProcess<ISleep>> {
    private:
    public:
      template <typename Particle, typename Track>
      EProcessReturn DoContinuous(Particle&, Track const&) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return process::EProcessReturn::eOk;
      }

      template <typename Particle, typename Track>
      units::si::LengthType MaxStepLength(Particle const& p, Track const& track) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return units::si::meter * std::numeric_limits<double>::infinity();
      }

      std::string name() { return "DummyContinuousProcess"; }
    };

  } // namespace devtools
} // namespace corsika::process