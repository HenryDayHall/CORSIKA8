/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_process_processsignature_h_
#define _include_process_processsignature_h_

#define FORCE_SIGNATURE(nameTrait, nameMethod, signatureMethod)                \
  template <typename U>                                                        \
  class nameTrait {                                                            \
  private:                                                                     \
    template <typename T, T>                                                   \
    struct helper;                                                             \
    template <typename T>                                                      \
    static std::uint8_t check(helper<signatureMethod, &nameMethod>*);          \
    template <typename T>                                                      \
    static std::uint16_t check(...);                                           \
                                                                               \
  public:                                                                      \
    static constexpr bool value = sizeof(check<U>(0)) == sizeof(std::uint8_t); \
  }

// FORCE_SIGNATURE(thisMustBeDefined, T::thisMustBeDefined, int(*)(void));

#endif
