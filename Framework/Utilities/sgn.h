
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _utilities_sgn_h
#define _utilities_sgn_h

namespace corsika::utl {

  //! sign function without branches
  template <typename T>
  static int sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

} // namespace corsika::utl

#endif
