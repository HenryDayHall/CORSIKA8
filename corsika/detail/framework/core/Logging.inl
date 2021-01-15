/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <spdlog/fmt/ostr.h> // will output whenerver a streaming operator is found
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace corsika {

  // many of these free functions are special to the logging
  // infrastructure so we hide them in the corsika::logging namespace.
  namespace logging {

    /*
     * The default pattern for CORSIKA8 loggers.
     */
    std::string const default_pattern{"[%n:%^%-8l%$] %v"};

    inline auto set_default_level(level::level_enum const minlevel) -> void {
      spdlog::set_level(minlevel);
    }

    template <typename TLogger>
    inline auto add_source_info(TLogger& logger) -> void {
      logger->set_pattern("[%n:%^%-8l%$(%s:%!:%#)] %v");
    }

    template <typename TLogger>
    inline auto reset_pattern(TLogger& logger) -> void {
      logger->set_pattern(default_pattern);
    }

  } // namespace logging

  inline std::shared_ptr<spdlog::logger> create_logger(std::string const& name,
                                                       bool const defaultlog) {

    // create the logger
    // this is currently a colorized multi-threading safe logger
    auto logger = spdlog::stdout_color_mt(name);

    // set the default C8 format
    logger->set_pattern(logging::default_pattern);

    // if defaultlog is True, we set this as the default spdlog logger.
    if (defaultlog) { spdlog::set_default_logger(logger); }

    return logger;
  }

  inline std::shared_ptr<spdlog::logger> get_logger(std::string const& name,
                                                    bool const defaultlog) {

    // attempt to get the logger from the registry
    auto logger = spdlog::get(name);

    // weg found the logger, so just return it
    if (logger) {
      return logger;
    } else { // logger was not found so create it
      return create_logger(name, defaultlog);
    }
  }

} // namespace corsika
