/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/InteractionProcess.h>

#include <chrono>
#include <thread>

namespace corsika::process {
  namespace example_processors {

    template <int ISleep>
    class DummyInteractionProcess : InteractionProcess<DummyInteractionProcess<ISleep> > {
    private:
    public:
      template <typename Particle>
      EProcessReturn DoInteraction(Particle&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return process::EProcessReturn::eOk;
      }

      template <typename TParticle>
      corsika::units::si::GrammageType GetInteractionLength(TParticle&) {
        using namespace corsika::units::si;

        std::this_thread::sleep_for(std::chrono::milliseconds(ISleep));
        return std::numeric_limits<double>::infinity() * (1_g / 1_cm / 1_cm);
      }
    };

  } // namespace example_processors
} // namespace corsika::process