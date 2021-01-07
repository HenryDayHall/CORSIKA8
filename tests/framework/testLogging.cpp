/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/Logging.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("Logging", "[Logging]") {
  SECTION("top level functions using default corsika logger") {
    logging::info("This is an info message!");
    logging::warn("This is a warning message!");
    logging::debug("This is a debug message!");
    logging::error("This is an error message!");
    logging::critical("This is a critical error message!");
  }

  SECTION("create a specific logger") {

    // create a logger manually
    auto logger = create_logger("loggerA");

    // set a custom pattern for this logger
    logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

    // and make sure we can log with this created object
    logger->info("This is an info message!");
    logger->warn("This is a warning message!");
    logger->debug("This is a debug message!");
    logger->error("This is an error message!");
    logger->critical("This is a critical error message!");

    // get a reference to the logger using Get
    auto other = get_logger("loggerA");

    // and make sure we can use this other reference to log
    other->info("This is an info message!");
    other->warn("This is a warning message!");
    other->debug("This is a debug message!");
    other->error("This is an error message!");
    other->critical("This is a critical error message!");
  }

  SECTION("get a new logger") {

    // get a reference to an unknown logger
    auto logger = get_logger("loggerB");

    // and make sure we can log with this created object
    logger->info("This is an info message!");
    logger->warn("This is a warning message!");
    logger->debug("This is a debug message!");
    logger->error("This is an error message!");
    logger->critical("This is a critical error message!");
  }

  SECTION("test log level") {

    // set the default log level
    logging::set_default_level(logging::level::critical);

    // and make sure we can log with this created object
    logging::info("This should NOT be printed!");
    logging::warn("This should NOT be printed!");
    logging::debug("This should NOT be printed!");
    logging::error("This should NOT be printed!");
    logging::critical("This SHOULD BE printed!!");

    // get a reference to an unknown logger
    auto logger = get_logger("loggerD");

    // now set the default log level for this logger
    logger->set_level(logging::level::critical);

    // now try the various logging functions
    logger->info("This should NOT be printed!");
    logger->warn("This should NOT be printed!");
    logger->debug("This should NOT be printed!");
    logger->error("This should NOT be printed!");
    logger->critical("This SHOULD BE printed!!");

    // and reset it for the next tests
    logging::set_default_level(logging::level::debug);
    logger->set_level(logging::level::critical);
  }

  SECTION("test macro style logging") {

    // these print with the "corsika" logger
    CORSIKA_LOG_INFO("test macro style logging");
    CORSIKA_LOG_DEBUG("test macro style logging");
    CORSIKA_LOG_ERROR("test macro style logging");
    CORSIKA_LOG_CRITICAL("test macro style logging");

    // get a reference to an unknown logger
    auto logger = get_logger("loggerE");

    // add the filename, source information to this logger
    logging::add_source_info(logger);

    // these print with the "loggerE" logger
    CORSIKA_LOGGER_INFO(logger, "test macro style logging");
    CORSIKA_LOGGER_WARN(logger, "test macro style logging");

    // reset the logging pattern
    logging::reset_pattern(logger);

    // these trace macros should not print file, function, and line
    CORSIKA_LOGGER_TRACE(logger, "test macro style logging:");
  }
}
