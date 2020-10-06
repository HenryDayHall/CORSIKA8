/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/DecayProcess.h>

#include <corsika/process/analytic_processors/ExecTime.h>

namespace corsika::process {
  namespace analytic_processors {
    template <typename T>
    class _ExecTimeImpl;

    template <class T, bool TCheck>
    class Decay;

    template <class T>
    class Decay<T, false> {};

    template <class T>
    class Decay<T, true> : public _ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle>
      EProcessReturn DoDecay(Particle& p) {
        this->start();
        auto r = _ExecTimeImpl<T>::DoDecay(p);
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
    };
  } // namespace analytic_processors
} // namespace corsika::process