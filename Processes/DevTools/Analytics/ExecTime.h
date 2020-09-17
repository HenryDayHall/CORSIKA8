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

    template <typename T, bool>
    class ExecTime_BoundaryCrossing {};

    template <typename T>
    class ExecTime_BoundaryCrossing<T, true> : protected T {
    public:
      template <
          typename Particle, typename VTNType,
          typename std::enable_if_t<
              std::is_base_of<BoundaryCrossingProcess<typename T::_TDerived>, T>::value,
              int> = 0>
      EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                        VTNType const& to) {
        return T::DoBoundaryCrossing(p, from, to);
      }
    };

    template <typename T, bool>
    class ExecTime_Continuous {};

    template <typename T>
    class ExecTime_Continuous<T, true> : protected T {
    public:
      template <typename Particle, typename Track>
      EProcessReturn DoContinuous(Particle& p, Track const& t) const {
        return T::DoContinous(p, t);
      }

      template <typename Particle, typename Track>
      units::si::LengthType MaxStepLength(Particle const& p, Track const& track) const {
        return T::MaxStepLength(p, track);
      }
    };

    template <typename T, bool>
    class ExecTime_Decay {};

    template <typename T>
    class ExecTime_Decay<T, true> : protected T {
    public:
      template <typename Particle>
      EProcessReturn DoDecay(Particle& p) {
        return T::DoDecay(p);
      }

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle& p) {
        return T::GetLifetime(p);
      }
    };

    template <typename T, bool>
    class ExecTime_Interaction {};

    template <typename T>
    class ExecTime_Interaction<T, true> : protected T {
    public:
      template <typename Particle>
      EProcessReturn DoInteraction(Particle& p) {
        return T::DoInteraction(p);
      }

      template <typename Particle>
      corsika::units::si::GrammageType GetInteractionLength(Particle& p) {
        return T::GetInteractionLength(p);
      }
    };

    template <typename T, bool>
    class ExecTime_Secondaries {};

    template <typename T>
    class ExecTime_Secondaries<T, true> : protected T {
    public:
      template <typename Secondaries>
      inline EProcessReturn DoSecondaries(Secondaries& sec) {
        return T::DoSecondaries(sec);
      }
    };

    template <typename T>
    class ExecTime
        : public ExecTime_BoundaryCrossing<
              T,
              std::is_base_of<BoundaryCrossingProcess<typename T::_TDerived>, T>::value>,
          public ExecTime_Continuous<
              T, std::is_base_of<ContinuousProcess<typename T::_TDerived>, T>::value>,
          public ExecTime_Decay<T,
                         std::is_base_of<DecayProcess<typename T::_TDerived>, T>::value>,
          public ExecTime_Interaction<
              T, std::is_base_of<InteractionProcess<typename T::_TDerived>, T>::value>,
          public ExecTime_Secondaries<
              T, std::is_base_of<SecondariesProcess<typename T::_TDerived>, T>::value> {
    private:
      void start();
      void stop();

    protected:
    public:
      float mean();
      float min();
      float max();
      float var();

      /*
                  // Stack
                  template <
                      typename TStack,
                      typename std::enable_if_t<
                          std::is_base_of<StackProcess<typename T::_TDerived>, T>::type,
         int> = 0> inline EProcessReturn DoStack(TStack& stack) { return T::stack(stack);
                  }*/
    };
  } // namespace devtools
} // namespace corsika::process