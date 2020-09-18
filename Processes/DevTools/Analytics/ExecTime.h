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

    template <typename T>
    class ExecTime : private T {
    private:
      std::chrono::high_resolution_clock::time_point fStart;
      std::chrono::duration<double,std::micro> fElapsedSum;
      double fMean;
      double fMean2;
      double fMin;
      double fMax;
      long long fN;

    protected:
    public:
      ExecTime() {
        fMin = std::numeric_limits<long long>::max();
        fMax = 0;
        fMean = 0;
        fMean2 = 0;
        fN = 0;
      }

      void start() { fStart = std::chrono::high_resolution_clock::now(); }
      void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::micro> timeDiv =
            std::chrono::duration_cast< std::chrono::duration<double,std::micro> >(end - fStart);

        fElapsedSum += timeDiv;
        fN = fN + 1;

        if (fMax < timeDiv.count()) fMax = timeDiv.count();

        if (timeDiv.count() < fMin) fMin = timeDiv.count();

        double delta = timeDiv.count() - fMean;
        fMean += delta / static_cast<double>(fN);

        double delta2 = timeDiv.count() - fMean;

        fMean2 += delta * delta2;
      }

      double mean() const { return fMean; }
      double min() const { return fMin; }
      double max() const { return fMax; }
      double var() const { return fMean2 / fN; }
      double sumTime() const { return fElapsedSum.count(); }

      template <typename Particle, typename VTNType>
      EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                        VTNType const& to) {
        this->start();
        auto r = T::DoBoundaryCrossing(p, from, to);
        this->stop();
        return r;
      }

      template <typename Particle, typename Track>
      EProcessReturn DoContinuous(Particle& p, Track const& t) const {
        this->start();
        auto r = T::DoContinous(p, t);
        this->stop();
        return r;
      }

      template <typename Particle, typename Track>
      units::si::LengthType MaxStepLength(Particle const& p, Track const& track) const {
        this->start();
        auto r = T::MaxStepLength(p, track);
        this->stop();
        return r;
      }

      template <typename Particle>
      EProcessReturn DoDecay(Particle& p) {
        this->start();
        auto r = T::DoDecay(p);
        this->stop();
        return r;
      }

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle& p) {
        this->start();
        auto r = T::GetLifetime(p);
        this->stop();
        return r;
      }

      template <typename Particle>
      EProcessReturn DoInteraction(Particle& p) {
        this->start();
        auto r = T::DoInteraction(p);
        this->stop();
        return r;
      }

      template <typename Particle>
      corsika::units::si::GrammageType GetInteractionLength(Particle& p) {
        this->start();
        auto r = T::GetInteractionLength(p);
        this->stop();
        return r;
      }

      template <typename Secondaries>
      inline EProcessReturn DoSecondaries(Secondaries& sec) {
        this->start();
        auto r = T::DoSecondaries(sec);
        this->stop();
        return r;
      }

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