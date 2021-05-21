/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

/**
 * @file ImplementsMixin.hpp
 */

#include <type_traits>

namespace corsika {

  namespace detail {

    /**
       Helper traits class (partial) for static compile time checking.

       This method checks whether a given class implements a particular
       mixin - this is particularly useful in the media heirarchy that utilizes
       mixins for the interface definition.
    */
    template <template <typename> typename Mixin, typename T>
    struct implements_mixin {

      template <typename U>
      static std::true_type test(Mixin<U>&);
      static std::false_type test(...);

    public:
      using type = decltype(test(std::declval<T&>()));
      static constexpr bool value = type::value;
    };

  } // namespace detail
} // namespace corsika
