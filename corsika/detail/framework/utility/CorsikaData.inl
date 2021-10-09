/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/framework/core/Logging.hpp>

#include <boost/filesystem/path.hpp>

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace corsika {

  inline boost::filesystem::path corsika_data(boost::filesystem::path const& key) {
    std::string const corsika_Data_Dir = std::string(CORSIKA_DATA_DIR); // from cmake
    boost::filesystem::path fname = boost::filesystem::path(corsika_Data_Dir) / key;
    // LCOV_EXCL_START, this cannot be easily tested system-independently
    if (auto const* p = std::getenv("CORSIKA_DATA"); p != nullptr) {
      fname = boost::filesystem::path(p) / key;
    }
    // LCOV_EXCL_STOP
    CORSIKA_LOG_INFO("opening data file={}", fname);
    return fname;
  }
} // namespace corsika
