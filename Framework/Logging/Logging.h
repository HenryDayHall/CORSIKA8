/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
 *  @File Logging.h
 *
 * CORSIKA8 logging utilities.
 *
 * See testLogging.cc for a complete set of examples for
 * how the logging functions should be used.
 */

#pragma once

// Configure some behaviour of sdlog.
// This must be done before spdlog is included.

// use the coarse system clock. This is *much* faster
// but introduces a real time error of O(10 ms).
#define SPDLOG_CLOCK_COARSE

// do not create a default logger (we provide our own "corsika" logger)
#define SPDLOG_DISABLE_DEFAULT_LOGGER

// use __PRETTY_FUNCTION__ instead of __FUNCTION__ where
// printing function names in trace statements. This is much
// nicer than __FUNCTION__ under GCC/clang.
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__

// if this is a Debug build, include debug messages in objects
#ifdef DEBUG
// trace is the highest level of logging (ALL messages will be printed)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else // otherwise, remove everything but "critical" messages
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_CRITICAL
#endif

#include <spdlog/fmt/ostr.h> // will output whenerver a streaming operator is found
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace corsika::logging {

  // bring spdlog into the corsika::logging namespace
  using namespace spdlog;

  /*
   * The default pattern for CORSIKA8 loggers.
   */
  const std::string default_pattern{"[%n:%^%-8l%$] %v"};

  /**
   * Create a new C8-style logger.
   *
   * Use this if you are explicitly (and can guarantee) that you
   * are creating a logger for the first time. It is recommended
   * that for regular usage, the `GetLogger` function is used instead
   * as that will also create the logger if it has not yet been created.
   *
   * Calling `CreateLogger` twice to create the same logger will
   * result in an spdlog duplicate exception.
   *
   * @param name           The unique name of the logger.
   * @param defaultlog     If True, set this as the default logger.
   * @returns              The constructed and formatted logger.
   */
  inline auto CreateLogger(std::string const& name, bool const defaultlog = false) {

    // create the logger
    // this is currently a colorized multi-threading safe logger
    auto logger = spdlog::stdout_color_mt(name);

    // set the default C8 format
    logger->set_pattern(default_pattern);

    // if defaultlog is True, we set this as the default spdlog logger.
    if (defaultlog) { spdlog::set_default_logger(logger); }

    // and return the logger
    return logger;
  }

  /**
   * Get a smart pointer to an existing logger.
   *
   * This should be the default method for code to obtain a
   * logger. If the logger *does not* exist, it is *created* and
   * returned to the caller.
   *
   * This should be preferred over `CreateLogger`.
   *
   * @param name    The name of the logger to get.
   * @param defaultlog   If True, make this the default logger.
   * @returns              The constructed and formatted logger.
   */
  inline auto GetLogger(std::string const& name, bool const defaultlog = false) {

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
   * The default "corsika" logger.
   */
  inline auto corsika = GetLogger("corsika", true);

  /**
   * Set the default log level for all *newly* created loggers.
   *
   *  @param name    The minimum log level required to print.
   *
   */
  inline auto SetDefaultLevel(level::level_enum const minlevel) -> void {
    spdlog::set_level(minlevel);
  }

  /**
   * Set the log level for the "corsika" logger.
   *
   * @param name    The minimum log level required to print.
   *
   */
  inline auto SetLevel(level::level_enum const minlevel) -> void {
    corsika->set_level(minlevel);
  }

  /**
   * Set the log level for a specific logger.
   *
   * @param logger  The logger to set the level of.
   * @param name    The minimum log level required to print.
   *
   */
  template <typename TLogger>
  inline auto SetLevel(TLogger& logger, level::level_enum const minlevel) -> void {
    logger->set_level(minlevel);
  }

  /**
   * Add the source (filename, line no) info to the logger.
   *
   * @param logger  The logger to set the level of.
   *
   */
  template <typename TLogger>
  inline auto AddSourceInfo(TLogger& logger) -> void {
    logger->set_pattern("[%n:%^%-8l%$(%s:%!:%#)] %v");
  }

  /**
   * Reset the logging pattern to the default.
   *
   * @param logger  The logger to set the level of.
   *
   */
  template <typename TLogger>
  inline auto ResetPattern(TLogger& logger) -> void {
    logger->set_pattern(default_pattern);
  }

// define our macro-style loggers
#define C8LOG_TRACE SPDLOG_TRACE
#define C8LOG_DEBUG SPDLOG_DEBUG
#define C8LOG_INFO SPDLOG_INFO
#define C8LOG_WARN SPDLOG_WARN
#define C8LOG_ERROR SPDLOG_ERROR
#define C8LOG_CRITICAL SPDLOG_CRITICAL

// and the specific logger versions
#define C8LOG_LOGGER_TRACE SPDLOG_LOGGER_TRACE
#define C8LOG_LOGGER_DEBUG SPDLOG_LOGGER_DEBUG
#define C8LOG_LOGGER_INFO SPDLOG_LOGGER_INFO
#define C8LOG_LOGGER_WARN SPDLOG_LOGGER_WARN
#define C8LOG_LOGGER_ERROR SPDLOG_LOGGER_ERROR
#define C8LOG_LOGGER_CRITICAL SPDLOG_LOGGER_CRITICAL

} // namespace corsika::logging
