/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/DecayProcess.h>

#include <corsika/process/analytic_processors/ExecTimeImpl.h>

namespace corsika::process {
  namespace analytic_processors {

    namespace detail {
      template <typename T>
      class ExecTimeImpl;
    }

    /// Base for Decay Implementation
    template <class T, bool TCheck>
    class Decay;

    /// Specialisation if class is not DecayProcess
    template <class T>
    class Decay<T, false> {};

    /// Specialisation if class is a DecayProcess
    template <class T>
    class Decay<T, true> : public detail::ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle>
      EProcessReturn DoDecay(Particle& p) {
        this->start();
        auto r = detail::ExecTimeImpl<T>::DoDecay(p);
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