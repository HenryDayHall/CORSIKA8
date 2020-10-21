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

class _foo2 {
public:
  int inside(int) { return 123; }

  int inside(char) { return 312; }
};

class _foo1 : public _foo2 {
public:
  int inside(int) { return 123; }
};

class foo : public _foo1 {
public:
  int bar() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 31415;
  }

  void bar2(int) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  inline void bar_const() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }

  int inside() {
    auto tc = corsika::analytics::ClassTimer<int (_foo1::*)(int), &_foo1::inside>(*this);

    auto r = tc.call(1);

    return r;
  }
};

template <typename TType, TType>
class timeMin;

template <typename TType, typename TRet, typename... TArgs,
          TRet (TType::*TFuncPtr)(TArgs...)>
class timeMin<TRet (TType::*)(TArgs...), TFuncPtr> {
private:
  TType& obj_;

public:
  timeMin(TType& obj)
      : obj_(obj) {}

  TRet call(TArgs... args) { return (obj_.*TFuncPtr)(std::forward<TArgs>(args)...); }
};

// quasi processor
class fooT1 {
public:
  template <typename T1, typename T2>
  int inside_t(T1 a, T2 b, T2 c) {
    return 123;
  }
};

//  exec_time_impl
template <typename T>
class fooT2 : public T {
public:
  using _T = T;
};

//  exec_time_impl
template <typename T>
class fooT3 : public fooT2<T> {
public:
  template <typename T1, typename T2>
  int inside_t(T1 a, T2 b, T2 c) {
    auto tc =
        timeMin<int (fooT2<T>::_T::*)(T1, T2, T2),
                &fooT2<T>::_T::template inside_t<T1, T2>>(*this); // <- dependent template

    auto r = tc.call(a, b, c);

    return r;
  }
};

TEST_CASE("Analytics", "[Timer]") {
  SECTION("Measure runtime of a function without arguments") {

    auto test = foo();
    auto tc = corsika::analytics::ClassTimer<decltype(&foo::bar), &foo::bar>(test);

    tc.call();

    REQUIRE(tc.getTime().count() == Approx(100000).margin(1000));
  }

  SECTION("Measure runtime of a function with arguments") {

    auto test = foo();
    auto tc = corsika::analytics::ClassTimer<decltype(&foo::bar2), &foo::bar2>(test);

    tc.call(1);

    REQUIRE(tc.getTime().count() == Approx(100000).margin(1000));
  }

  SECTION("Measure runtime of a const function without arguments") {

    auto test = foo();
    auto tc =
        corsika::analytics::ClassTimer<decltype(&foo::bar_const), &foo::bar_const>(test);

    tc.call();

    REQUIRE(tc.getTime().count() == Approx(100000).margin(1000));
  }

  SECTION("Measure runtime of function inside class") {

    auto test = foo();
    REQUIRE(test.inside() == 123);
  }

  SECTION("Measure runtime of function inside class") {
    auto test = fooT3<fooT1>();
    REQUIRE(test.inside_t(1, 'a', 'b') == 123);
  }
}
