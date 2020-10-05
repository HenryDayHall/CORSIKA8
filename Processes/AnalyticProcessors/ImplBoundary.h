/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/process/analytic_processors/ExecTime.h>

namespace corsika::process {
  namespace analytic_processors {

    template <typename T>
    class _ExecTimeImpl;

    template <class T, bool TCheck>
    class Boundary;

    template <class T>
    class Boundary<T, false> {};

    template <class T>
    class Boundary<T, true> : public _ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle, typename VTNType>
      EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                        VTNType const& to) {
        this->start();
        auto r = T::DoBoundaryCrossing(p, from, to);
        this->stop();
        return r;
      }
    };
  } // namespace devtools
} // namespace corsika::process