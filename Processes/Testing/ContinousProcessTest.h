/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

TEMPLATE_TEST_CASE("vectors can be sized and resized", "[vector][template]", int,
                   std::string, (std::tuple<int, float>)) {

  std::vector<TestType> v(5);

  REQUIRE(v.size() == 5);
  REQUIRE(v.capacity() >= 5);

  SECTION("resizing bigger changes size and capacity") {
    v.resize(10);

    REQUIRE(v.size() == 10);
    REQUIRE(v.capacity() >= 10);
  }
  SECTION("resizing smaller changes size but not capacity") {
    v.resize(0);

    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() >= 5);

    SECTION("We can use the 'swap trick' to reset the capacity") {
      std::vector<TestType> empty;
      empty.swap(v);

      REQUIRE(v.capacity() == 0);
    }
  }
  SECTION("reserving smaller does not change size or capacity") { v.reserve(0); }
}
