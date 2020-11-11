/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/NullModel.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

/*
 * The NullModel can do really nothing, so we can basically test
 * nothing.
 */

TEST_CASE("NullModel", "[processes]") {

  SECTION("interface") { [[maybe_unused]] NullModel model; }
}
