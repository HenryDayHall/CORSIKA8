
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_continuousprocess_h_
#define _include_corsika_continuousprocess_h_

#include <corsika/process/ProcessReturn.h> // for convenience

namespace corsika::process {

  /**
     \class ContinuousProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type ContinuousProcess<T>

   */

  template <typename derived>
  struct ContinuousProcess {
    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }
  };

} // namespace corsika::process

#endif
