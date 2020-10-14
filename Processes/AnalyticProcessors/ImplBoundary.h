/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/process/analytic_processors/ExecTimeImpl.h>

#include <corsika/analytics/ClassTimer.h>

namespace corsika::process {
  namespace analytic_processors {

    namespace detail {
      template <typename T>
      class ExecTimeImpl;
    }

    template <class T, bool TCheck>
    class Boundary;

    template <class T>
    class Boundary<T, false> {};

    template <class T>
    class Boundary<T, true> : public detail::ExecTimeImpl<T> {
    private:
    public:
      template <typename Particle, typename VTNType>
      EProcessReturn DoBoundaryCrossing(Particle& p, VTNType const& from,
                                        VTNType const& to) {
        auto tc = corsika::analytics::ClassTimer<
            EProcessReturn (detail::ExecTimeImpl<T>::_T::*)(Particle&, VTNType const&,
                                                    VTNType const&),
            &detail::ExecTimeImpl<T>::_T::template DoBoundaryCrossing<Particle, VTNType>>(*this);

        EProcessReturn r = tc.call(p, from, to);
        this->update(
            std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(
                tc.getTime()));
        return r;
      }
    };
  } // namespace analytic_processors
} // namespace corsika::process