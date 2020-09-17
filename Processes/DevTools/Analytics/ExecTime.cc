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

    template <class T>
    void ExecTime<T>::start() {
      fStart = std::chrono::steady_clock::now();
    }

    template <class T>
    void ExecTime<T>::stop() {
      auto end = std::chrono::steady_clock::now();
      std::chrono::microseconds timeDiv =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);

      fElapsedSum += timeDiv;
      fN++;

      if (fMax < timeDiv) fMax = timeDiv;

      if (timeDiv < fMin) fMin = timeDiv;

      auto delta = timeDiv - fMean;
      fMean += delta / fN;

      auto delta2 = timeDiv - fMean;

      fMean2 += delta * delta2;
    }

    template <typename T>
    double ExecTime<T>::mean() {
      return fMean;
    }

    template <typename T>
    double ExecTime<T>::var() {
      return fMean2 / count;
    }

    template <typename T>
    double ExecTime<T>::min() {
      return fMin;
    }

    template <typename T>
    double ExecTime<T>::max() {
      return fMax;
    }

  } // namespace devtools
} // namespace corsika::process