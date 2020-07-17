/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/utl/CorsikaData.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

std::string corsika::utl::CorsikaData(std::string const& key) {
  if (auto const* p = std::getenv("CORSIKA_DATA"); p != nullptr) {
    auto const path = std::string(p) + "/" + key;
    return path;
  } else {
    throw std::runtime_error("CORSIKA_DATA not set");
  }
}
