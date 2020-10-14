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

#include <corsika/logging/Logging.h>

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

    namespace detail {

      template <typename T>
      class ExecTimeImpl : protected T {
      private:
        std::chrono::high_resolution_clock::time_point startTime_;
        std::chrono::duration<double, std::micro> cumulatedTime_;
        volatile double mean_;
        volatile double mean2_;
        volatile double min_;
        volatile double max_;
        volatile long long n_;

      protected:
      public:
        using _T = T;

        ExecTimeImpl() {
          min_ = std::numeric_limits<double>::max();
          cumulatedTime_ = std::chrono::duration<double, std::micro>(0);
          max_ = 0;
          mean_ = 0;
          mean2_ = 0;
          n_ = 0;
        }

        inline void start() { startTime_ = std::chrono::high_resolution_clock::now(); }
        inline void stop() {
          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double, std::micro> timeDiv =
              std::chrono::duration_cast<std::chrono::duration<double, std::micro> >(
                  end - startTime_);

          this->update(timeDiv);
        }

        void update(std::chrono::duration<double, std::micro> timeDif) {

          cumulatedTime_ += timeDif;
          n_ = n_ + 1;

          if (max_ < timeDif.count()) max_ = timeDif.count();

          if (timeDif.count() < min_) min_ = timeDif.count();

          double delta = timeDif.count() - mean_;
          mean_ += delta / static_cast<double>(n_);

          double delta2 = timeDif.count() - mean_;

          mean2_ += delta * delta2;
        }

        inline double mean() const { return mean_; }
        inline double min() const { return min_; }
        inline double max() const { return max_; }
        inline double var() const { return mean2_ / n_; }
        inline double sumTime() const { return cumulatedTime_.count(); }
      };

    } // namespace detail

  } // namespace analytic_processors
} // namespace corsika::process