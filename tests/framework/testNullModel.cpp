/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/NullModel.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

/*
 * The NullModel can do really nothing, so we can basically test
 * nothing.
 */

TEST_CASE("NullModel", "[processes]") {

  logging::set_level(logging::level::info);

  SECTION("interface") {
    [[maybe_unused]] NullModel nm;

    CHECK(is_process_v<decltype(nm)>);
    CHECK(count_processes<decltype(nm)>::count == 0);
  }
}
