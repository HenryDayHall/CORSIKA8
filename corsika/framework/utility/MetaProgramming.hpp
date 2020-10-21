n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <type_traits>

/**
  \file MetaProgramming.h

  Tools to emulate conditional compilation of method overloads.

  ```
  #include <corsika/framework/utility/MetaProgramming.h>

  template <class T>
  struct Foo {
    template <class U, class = corsika::enable_if<std::reference<U>>>
    void enable_only_if_U_is_reference(U x) {}

    template <class _ = T, class = corsika::disable_if<std::reference<_>>>
    void enable_only_if_T_is_not_reference() {}
  };
  ```

  To combine traits, use boost::mp11::mp_and or boost::mp11::mp_or.

  ```
  #include <corsika/framework/utility/MetaProgramming.h>
  #include <boost/mp11.hpp>

  template <class T>
  struct Foo {
    template <class U, class = corsika::enable_if<
      boost::mp11::mp_and<std::is_reference<T>, std::is_const<T>>
    >
    void enable_only_if_U_is_const_reference(U x) {}
  };
  ```

  Common combined traits may be implemented in this header. For example:
  ```
  template <class T>
  using is_cref = boost::mp11::mp_and<std::is_reference<T>, std::is_const<T>>;
  ```
}
*/

namespace corsika {
  template <class Trait>
  using enable_if = typename std::enable_if<(Trait::value == true)>::type;

  template <class Trait>
  using disable_if = typename std::enable_if<(Trait::value == false)>::type;
} // namespace corsika
