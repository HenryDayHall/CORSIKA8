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
    void ExecTime<T>::start() {}

    template <class T>
    void ExecTime<T>::stop() {}

  } // namespace devtools
} // namespace corsika::process