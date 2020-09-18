/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <chrono>
#include <iostream>

#include <corsika/process/devtools/ExecTime.h>

namespace corsika::process {
  namespace devtools {

 

  } // namespace devtools
} // namespace corsika::process

/*
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
    */