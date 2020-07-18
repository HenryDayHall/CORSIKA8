/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @File Logging.h

   CORSIKA8 logging utilities.
 */

#pragma once

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"


namespace corsika::logging {

  // bring spdlog into the corsika::logging namespace
  using namespace spdlog;

  /**
   * Create a new C8-style logger.
   *
   * @param name           The unique name of the logger.
   * @param defaultlog     If True, set this as the default logger.
   * @returns              The constructed and formatted logger.
   */
  auto CreateLogger(std::string const& name, bool const defaultlog = false) {

    // create the logger
    auto logger = spdlog::stdout_color_mt(name);

    // set the default C8 format
    logger->set_pattern("[%n:%^%-8l%$] %v");

    // if defaultlog is True, we set this as the default spdlog logger.
    if (defaultlog) { spdlog::set_default_logger(logger); }

    // and return the logger
    return logger;
  }

  /**
   * Get a reference to a named logger.
   *
   * If the logger does not exist, it is created.
   *
   * @param name    The name of the logger to get.
   */
  auto GetLogger(std::string const& name, bool const defaultlog = false) {

    // attempt to get the logger from the registry
    auto logger = spdlog::get(name);

    // weg found the logger, so just return it
    if (logger) {
      return logger;
    } else { // logger was not found so create it
      return CreateLogger(name, defaultlog);
    }
  }

  /**
   * Set the default log level for all loggers.
   *
   * @param name    The minimum log level required to print.
   *
   */
  auto SetDefaultLevel(level::level_enum minlevel) -> void {
    spdlog::set_level(minlevel);
  }

  // create the default CORSIKA logger
  inline auto corsika = GetLogger("corsika", true);

// define our macro-style loggers
#define C8LOG_DEBUG SPDLOG_DEBUG
#define C8LOG_INFO SPDLOG_INFO
#define C8LOG_WARN SPDLOG_WARN
#define C8LOG_ERROR SPDLOG_ERROR
#define C8LOG_CRITICAL SPDLOG_CRITICAL

// and the specific logger versions
#define C8LOG_LOGGER_DEBUG SPDLOG_LOGGER_DEBUG
#define C8LOG_LOGGER_INFO SPDLOG_LOGGER_INFO
#define C8LOG_LOGGER_WARN SPDLOG_LOGGER_WARN
#define C8LOG_LOGGER_ERROR SPDLOG_LOGGER_ERROR
#define C8LOG_LOGGER_CRITICAL SPDLOG_LOGGER_CRITICAL

} // namespace corsika::logging
