/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <filesystem>
#include <string>

namespace corsika::utl {
  /**
   * returns the full path of the file \p filename within the CORSIKA_DATA directory
   */
  std::filesystem::path CorsikaData(std::filesystem::path const& filename);
} // namespace corsika::utl
