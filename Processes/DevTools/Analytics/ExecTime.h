/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <chrono>
#include <type_traits>

#include <corsika/process/BoundaryCrossingProcess.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DecayProcess.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/process/StackProcess.h>

namespace corsika::process {
  namespace devtools {

    template <class T>
    class ExecTime : public T {
    private:
      void start();
      void stop();

    protected:
    public:

      float mean();
      float min();
      float max();
      float var();

      /// Interface implementation

      // Boundary Crossing
      template <
          typename Particle, typename VTNType,
          typename std::enable_if_t<
              std::is_base_of<BoundaryCrossingProcess<typename T::TDerived>, T>::type, int> = 0 >
      EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                        VTNType const& to) {
        return T::DoBoundaryCrossing(p, from, to);
      }

      // Continous
      template <typename Particle, typename Track,
                typename std::enable_if_t<
                    std::is_base_of<ContinuousProcess<typename T::TDerived>, T>::type, int> = 0>
      EProcessReturn DoContinuous(Particle& p, Track const& t) const {
        return T::DoContinous(p, t);
      }

      template <typename Particle, typename Track,
                typename std::enable_if_t<
                    std::is_base_of<ContinuousProcess<typename T::TDerived>, T>::type, int> = 0>
      units::si::LengthType MaxStepLength(Particle const& p, Track const& track) const {
        return T::MaxStepLength(p, track);
      }

      // Decay
      template <typename Particle,
                typename std::enable_if_t<
                    std::is_base_of<DecayProcess<typename T::TDerived>, T>::type, int> = 0>
      EProcessReturn DoDecay(Particle& p) {
        return T::DoDecay(p);
      }

      template <typename Particle,
                typename std::enable_if_t<
                    std::is_base_of<DecayProcess<typename T::TDerived>, T>::type, int> = 0>
      corsika::units::si::TimeType GetLifetime(Particle& p) {
        return T::GetLifetime(p);
      }

      // Interaction
      template <typename Particle,
                typename std::enable_if_t<
                    std::is_base_of<InteractionProcess<typename T::TDerived>, T>::type, int> = 0>
      EProcessReturn DoInteraction(Particle& p) {
        return T::DoInteraction(p);
      }

      template <typename TParticle,
                typename std::enable_if_t<
                    std::is_base_of<InteractionProcess<typename T::TDerived>, T>::type, int> = 0>
      corsika::units::si::GrammageType GetInteractionLength(TParticle& p) {
        return T::GetInteractionLength(p);
      }

      // Secondaries
      template <typename TSecondaries,
                typename std::enable_if_t<
                    std::is_base_of<SecondariesProcess<typename T::TDerived>, T>::type, int> = 0>
      inline EProcessReturn DoSecondaries(TSecondaries& sec) {
        return T::DoSecondaries(sec);
      }

      // Stack
      template <typename TStack,
                typename std::enable_if_t<
                    std::is_base_of<StackProcess<typename T::TDerived>, T>::type, int> = 0>
      inline EProcessReturn DoStack(TStack& stack) {
        return T::stack(stack);
      }
    };
  } // namespace devtools
} // namespace corsika::process