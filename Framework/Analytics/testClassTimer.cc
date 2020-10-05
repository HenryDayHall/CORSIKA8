/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/analytics/ClassTimer.h>

#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace corsika;

class foo {
public:
  int bar() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 31415;
  }

  void bar2(int i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  inline void bar_const() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }
};

TEST_CASE("Analytics", "[Timer]") {
  SECTION("Measure runtime of a function without arguments") {

    auto test = foo();
    auto tc = corsika::analytics::timeClass<decltype(&foo::bar), &foo::bar>(test);

    tc.call();

    std::cout << tc.getTime().count() << std::endl;
  }

  SECTION("Measure runtime of a function with arguments") {

    auto test = foo();
    auto tc = corsika::analytics::timeClass<decltype(&foo::bar2), &foo::bar2>(test);

    tc.call(1);

    std::cout << tc.getTime().count() << std::endl;
  }

  SECTION("Measure runtime of a const function without arguments") {

    auto test = foo();
    auto tc =
        corsika::analytics::timeClass<decltype(&foo::bar_const), &foo::bar_const>(test);

    tc.call();

    std::cout << tc.getTime().count() << std::endl;
  }
}
