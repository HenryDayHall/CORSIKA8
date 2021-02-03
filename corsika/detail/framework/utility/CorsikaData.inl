/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>

inline std::string corsika::corsika_data(std::string const& key) {
  if (auto const* p = std::getenv("CORSIKA_DATA"); p != nullptr) {
    auto const path = std::string(p) + "/" + key;
    return path;
  } else {
    throw std::runtime_error("CORSIKA_DATA not set");
  }
}
