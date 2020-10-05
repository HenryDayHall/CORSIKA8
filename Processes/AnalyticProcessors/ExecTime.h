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

#include <corsika/process/analytic_processors/ImplBoundary.h>
#include <corsika/process/analytic_processors/ImplContinuous.h>
#include <corsika/process/analytic_processors/ImplDecay.h>
#include <corsika/process/analytic_processors/ImplInteraction.h>
#include <corsika/process/analytic_processors/ImplSecondaries.h>

namespace corsika::process {
  namespace analytic_processors {

    template <typename T>
    class _ExecTimeImpl : protected T {
    private:
      std::chrono::high_resolution_clock::time_point startTime_;
      std::chrono::duration<double, std::micro> cumulatedTime_;
      double mean_;
      double mean2_;
      double min_;
      double max_;
      long long n_;

    protected:
    public:
      _ExecTimeImpl() {
        min_ = std::numeric_limits<long long>::max();
        max_ = 0;
        mean_ = 0;
        mean2_ = 0;
        n_ = 0;
      }

      void start() { startTime_ = std::chrono::high_resolution_clock::now(); }
      void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> timeDiv =
            std::chrono::duration_cast<std::chrono::duration<double, std::micro> >(
                end - startTime_);

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
    };

    template <typename T>
    class ExecTime
        : public Boundary<T, std::is_base_of<corsika::process::BoundaryCrossingProcess<
                                                 typename T::_TDerived>,
                                             T>::value>,
          public Continuous<
              T,
              std::is_base_of<corsika::process::ContinuousProcess<typename T::_TDerived>,
                              T>::value>,
          public Decay<
              T, std::is_base_of<corsika::process::DecayProcess<typename T::_TDerived>,
                                 T>::value>,
          public Interaction<
              T,
              std::is_base_of<corsika::process::InteractionProcess<typename T::_TDerived>,
                              T>::value>,
          public Secondaries<
              T, std::is_base_of<corsika::process::SecondariesProcess<typename T::_TDerived>,
                                 T>::value> {};
  } // namespace devtools
} // namespace corsika::process