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
      std::chrono::high_resolution_clock::time_point startTime_;
      std::chrono::duration<double,std::micro> cumulatedTime_;
      double mean_;
      double mean2_;
      double min_;
      double max_;
      long long n_;

    protected:
    public:
      ExecTime() {
        min_ = std::numeric_limits<long long>::max();
        max_ = 0;
        mean_ = 0;
        mean2_ = 0;
        n_ = 0;
      }

      void start() { startTime_ = std::chrono::high_resolution_clock::now(); }
      void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::micro> timeDiv =
            std::chrono::duration_cast< std::chrono::duration<double,std::micro> >(end - startTime_);

        cumulatedTime_ += timeDiv;
        n_ = n_ + 1;

        if (max_ < timeDiv.count()) max_ = timeDiv.count();

        if (timeDiv.count() < min_) min_ = timeDiv.count();

        double delta = timeDiv.count() - mean_;
        mean_ += delta / static_cast<double>(n_);

        double delta2 = timeDiv.count() - mean_;

        mean2_ += delta * delta2;
      }

      double mean() const { return mean_; }
      double min() const { return min_; }
      double max() const { return max_; }
      double var() const { return mean2_ / n_; }
      double sumTime() const { return cumulatedTime_.count(); }

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