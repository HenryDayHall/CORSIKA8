/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/analytics/FunctionTimer.h>

#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace corsika;

int testFunc() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return 31415;
}

class TestClass {
public:

  int operator()() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 31415;
  }
};

TEST_CASE("Analytics", "[Timer]") {
  SECTION("Measure runtime of a free function") {

    auto test = corsika::analytics::FunctionTimer(testFunc);

    std::cout << test() << std::endl;
    std::cout << test.getTime().count() << std::endl;
  }

  SECTION("Measure runtime of a class functor") {
    TestClass testC;
    auto test = corsika::analytics::FunctionTimer(testC);

    std::cout << test() << std::endl;
    std::cout << test.getTime().count() << std::endl;
  }
 
}
