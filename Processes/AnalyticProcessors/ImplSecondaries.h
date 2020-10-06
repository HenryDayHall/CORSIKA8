/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once
#include <corsika/process/SecondariesProcess.h>

#include <corsika/process/analytic_processors/ExecTime.h>

namespace corsika::process {
  namespace analytic_processors {
    template <typename T>
    class _ExecTimeImpl;

    template <class T, bool TCheck>
    class Secondaries;

    template <class T>
    class Secondaries<T, false> {};

    template <class T>
    class Secondaries<T, true> : public _ExecTimeImpl<T> {
    private:
    public:
      template <typename Secondaries>
      inline EProcessReturn DoSecondaries(Secondaries& sec) {
        this->start();
        auto r = _ExecTimeImpl<T>::DoSecondaries(sec);
        this->stop();
        return r;
      }
    };
  } // namespace analytic_processors
} // namespace corsika::process