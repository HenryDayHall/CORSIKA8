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

  template <class AConstIterator, class BConstIterator>
  inline WeightProviderIterator<AConstIterator, BConstIterator>::WeightProviderIterator(
      AConstIterator a, BConstIterator b)
      : aIter_(a)
      , bIter_(b) {}

  template <class AConstIterator, class BConstIterator>
  inline typename WeightProviderIterator<AConstIterator, BConstIterator>::value_type
      WeightProviderIterator<AConstIterator, BConstIterator>::operator*() const {
    return ((*aIter_) * (*bIter_)).magnitude();
  }

  template <class AConstIterator, class BConstIterator>
  inline WeightProviderIterator<AConstIterator, BConstIterator>&
  WeightProviderIterator<AConstIterator,
                         BConstIterator>::operator++() { // prefix ++
    ++aIter_;
    ++bIter_;
    return *this;
  }

  template <class AConstIterator, class BConstIterator>
  inline bool WeightProviderIterator<AConstIterator, BConstIterator>::operator==(
      WeightProviderIterator other) {
    return aIter_ == other.aIter_;
  }

  template <class AConstIterator, class BConstIterator>
  inline bool WeightProviderIterator<AConstIterator, BConstIterator>::operator!=(
      WeightProviderIterator other) {
    return !(*this == other);
  }

} // namespace corsika
