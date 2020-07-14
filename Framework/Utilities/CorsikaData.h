/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#ifndef CORSIKA_CORSIKADATA_H
#define CORSIKA_CORSIKADATA_H

#include <string>

namespace corsika::utl {
  /**
   * returns the full path of the file \p filename within the CORSIKA_DATA directory
   */
  std::string CorsikaData(std::string const& filename);
} // namespace corsika::utl

#endif // CORSIKA_CORSIKADATA_H
