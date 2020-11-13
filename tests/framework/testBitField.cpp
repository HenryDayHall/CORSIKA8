/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/framework/utility/BitField.hpp>

using namespace corsika;

TEST_CASE("test of BitField handling") {

  SECTION("BitField based on integer") {
    unsigned short test = 0x1337;
    auto testBit = BitField(test);

    // 0b1001100110111

    REQUIRE(testBit[0] == 1);
    REQUIRE(testBit[1] == 1);
    REQUIRE(testBit[2] == 1);
    REQUIRE(testBit[3] == 0);
    REQUIRE(testBit[4] == 1);
    REQUIRE(testBit[5] == 1);
    REQUIRE(testBit[6] == 0);
    REQUIRE(testBit[7] == 0);
    REQUIRE(testBit[8] == 1);
    REQUIRE(testBit[9] == 1);
    REQUIRE(testBit[10] == 0);
    REQUIRE(testBit[11] == 0);
    REQUIRE(testBit[12] == 1);
    REQUIRE(testBit[13] == 0);
    REQUIRE(testBit[14] == 0);
    REQUIRE(testBit[15] == 0);
  }

  SECTION("BitField based on struct") {
      // This fails to compile, because BitField requires that TType is constructible from int
      /*
    struct testStruct {
      char tmp1[2];
      short tmp2;
    } test;

    REQUIRE(sizeof(test) == 4);
    auto testBit = BitField(test);

    // 0b1001100110111

    REQUIRE(testBit[0] == 1);
    REQUIRE(testBit[1] == 1);
    REQUIRE(testBit[2] == 1);
    REQUIRE(testBit[3] == 0);
    REQUIRE(testBit[4] == 1);
    REQUIRE(testBit[5] == 1);
    REQUIRE(testBit[6] == 0);
    REQUIRE(testBit[7] == 0);
    REQUIRE(testBit[8] == 1);
    REQUIRE(testBit[9] == 1);
    REQUIRE(testBit[10] == 0);
    REQUIRE(testBit[11] == 0);
    REQUIRE(testBit[12] == 1);
    REQUIRE(testBit[13] == 0);
    REQUIRE(testBit[14] == 0);
    REQUIRE(testBit[15] == 0);
    */
  }
}