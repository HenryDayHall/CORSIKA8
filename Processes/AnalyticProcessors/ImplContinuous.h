/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/ContinuousProcess.h>

#include <corsika/process/analytic_processors/ExecTimeImpl.h>

namespace corsika::process {
  namespace analytic_processors {

    namespace detail {
      template <typename T>
      class ExecTimeImpl;
    }

    /// Base for Continuous Implementation
    template <class T, bool TCheck>
    class Continuous;

    /// Specialisation if class is not ContinuousProcess
    template <class T>
    class Continuous<T, false> {};

    /// Specialisation if class is a ContinuousProcess
    template <class T>
    class Continuous<T, true> : public detail::ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle, typename Track>
      EProcessReturn DoContinuous(Particle& p, Track const& t) {
        this->start();
        auto r = detail::ExecTimeImpl<T>::DoContinuous(p, t);
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
  } // namespace analytic_processors
} // namespace corsika::process