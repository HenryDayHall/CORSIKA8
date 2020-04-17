/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/logging/Logger.h>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("Logging", "[Logging]") {
  SECTION("top level functions using corsika logger") {
    logging::info("This is an info message!");
    logging::warn("This is a warning message!");
    logging::debug("This is a debug message!");
    logging::error("This is an error message!");
    logging::critical("This is a critical error message!");
  }

  SECTION("create a specific logger") {

    // create a logger manually
    auto logger = logging::CreateLogger("loggerA");

    // set a custom pattern for this logger
    logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

    // and make sure we can log with this created object
    logger->info("This is an info message!");
    logger->warn("This is a warning message!");
    logger->debug("This is a debug message!");
    logger->error("This is an error message!");
    logger->critical("This is a critical error message!");

    // get a reference to the logger using Get
    auto other = logging::GetLogger("loggerA");

    // and make sure we can use this other reference to log
    other->info("This is an info message!");
    other->warn("This is a warning message!");
    other->debug("This is a debug message!");
    other->error("This is an error message!");
    other->critical("This is a critical error message!");
  }

  SECTION("get a new logger") {

    // get a reference to an unknown logger
    auto logger = logging::GetLogger("loggerB");

    // and make sure we can log with this created object
    logger->info("This is an info message!");
    logger->warn("This is a warning message!");
    logger->debug("This is a debug message!");
    logger->error("This is an error message!");
    logger->critical("This is a critical error message!");
  }

  SECTION("test log level") {

    // set the default log level
    logging::SetDefaultLevel(logging::level::critical);

    // and make sure we can log with this created object
    logging::info("This should NOT be printed!");
    logging::warn("This should NOT be printed!");
    logging::debug("This should NOT be printed!");
    logging::error("This should NOT be printed!");
    logging::critical("This SHOULD BE printed!!");

    // get a reference to an unknown logger
    auto logger = logging::GetLogger("loggerD");

    // now set the default log level for this logger
    logging::SetLevel(logger, logging::level::critical);

    // now try the various logging functions
    logger->info("This should NOT be printed!");
    logger->warn("This should NOT be printed!");
    logger->debug("This should NOT be printed!");
    logger->error("This should NOT be printed!");
    logger->critical("This SHOULD BE printed!!");

    // and reset it for the next tests
    logging::SetDefaultLevel(logging::level::debug);
    logging::SetLevel(logging::level::debug);
  }

  SECTION("test macro style logging") {

    // these print with the "corsika" logger
    C8LOG_INFO("test macro style logging");
    C8LOG_DEBUG("test macro style logging");
    C8LOG_ERROR("test macro style logging");
    C8LOG_CRITICAL("test macro style logging");

    // get a reference to an unknown logger
    auto logger = logging::GetLogger("loggerE");

    // add the filename, source information to this logger
    logging::AddSourceInfo(logger);

    // these print with the "loggerE" logger
    C8LOG_LOGGER_INFO(logger, "test macro style logging");
    C8LOG_LOGGER_WARN(logger, "test macro style logging");

    // reset the logging pattern
    logging::ResetPattern(logger);

    // these trace macros should not print file, function, and line
    C8LOG_LOGGER_TRACE(logger, "test macro style logging:");
  }
}
