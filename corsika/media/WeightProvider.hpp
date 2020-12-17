/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <vector>

namespace corsika {


  /** Double Iterator
   * Iterator that allowes the iteration of two individual lists at the same time. The
   *user needs to take care that booth lists have the same length.
   *  @tparam AConstIterator Iterator Type of the first list
   *  @tparam BConstIterator Iterator Type of the second list

    \todo TODO: replace with https://www.boost.org/doc/libs/1_74_0/libs/iterator/doc/zip_iterator.html or ranges zip
    \todo check resource allocation
   **/
  template <class AConstIterator, class BConstIterator>
  class WeightProviderIterator {
    AConstIterator aIter_;
    BConstIterator bIter_;

  public:
    using value_type = double;
    using iterator_category = std::input_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;

    WeightProviderIterator(AConstIterator a, BConstIterator b);

    value_type operator*() const;

    WeightProviderIterator& operator++();

    bool operator==(WeightProviderIterator other);

    bool operator!=(WeightProviderIterator other);
  };
} // namespace corsika

#include <corsika/detail/media/WeightProvider.inl>
