/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/framework/process/NullModel.hpp>

using namespace corsika;

TEST_CASE("NullModel", "[processes]") {

  SECTION("interface") {
    [[maybe_unused]] NullModel model; // nothing to test...
  }
}
