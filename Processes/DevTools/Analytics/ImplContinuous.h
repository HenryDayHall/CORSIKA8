/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/ContinuousProcess.h>

#include <corsika/process/devtools/ExecTime.h>

namespace corsika::process {
  namespace devtools {
    template <typename T>
    class _ExecTimeImpl;

    template <class T, bool TCheck>
    class Continuous;

    template <class T>
    class Continuous<T, false> {};

    template <class T>
    class Continuous<T, true> : public _ExecTimeImpl<T> {
    private:
    public:
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
    };
  } // namespace devtools
} // namespace corsika::process