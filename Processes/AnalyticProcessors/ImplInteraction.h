/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/InteractionProcess.h>

#include <corsika/process/analytic_processors/ExecTimeImpl.h>

namespace corsika::process {
  namespace analytic_processors {

    namespace detail {
      template <typename T>
      class ExecTimeImpl;
    }

    template <class T, bool TCheck>
    class Interaction;

    template <class T>
    class Interaction<T, false> {};

    template <class T>
    class Interaction<T, true> : public detail::ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle>
      EProcessReturn DoInteraction(Particle& p) {
        this->start();
        auto r = detail::ExecTimeImpl<T>::DoInteraction(p);
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
    };
  } // namespace analytic_processors
} // namespace corsika::process