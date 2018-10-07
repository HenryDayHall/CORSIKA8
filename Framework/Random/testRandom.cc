
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <corsika/random/RNGManager.h>
#include <iostream>

using namespace corsika::random;

SCENARIO("random-number streams can be registered and retrieved") {
  GIVEN("a RNGManager") {
    RNGManager rngManager;

    WHEN("a sequence is registered by name") {
      rngManager.RegisterRandomStream("stream_A");

      THEN("the sequence can be retrieved") {
        REQUIRE_NOTHROW(rngManager.GetRandomStream("stream_A"));
      }

      THEN("an unknown sequence cannot be retrieved") {
        REQUIRE_THROWS(rngManager.GetRandomStream("stream_UNKNOWN"));
      }
    }
  }
}
