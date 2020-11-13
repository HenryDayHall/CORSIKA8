/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  template <typename TType>
  BitField<TType>::BitField(TType& target)
      : target_(target) {}

  template <typename TType>
  typename BitField<TType>::Bit BitField<TType>::operator[](unsigned int position) {
    return Bit(target_, TType(1) << position);
  }

  template <typename TType>
  typename BitField<TType>::Bit BitField<TType>::at(unsigned int position) {
    if (position >= 8 * sizeof(TType))
      // throw std::exceptionOutOfBoundException("Running out of bits.");
      throw std::exception("Running out of bits.");
    return (*this)[position];
  }

  template <typename TType>
  template <typename TMask>
  BitField<TType>& BitField<TType>::mask(const TMask mask, const bool value) {
    Bit(target_, mask) = value;
    return *this;
  }

  template <typename TType>
  template <typename TMask>
  TType BitField<TType>::get(const TMask mask) {
    return target_ & TType(mask);
  }

  // Nested Bit class:
  template <typename TType>
  BitField<TType>::Bit::Bit(TType& target, TType mask)
      : target_(target)
      , mask_(mask) {}

  template <typename TType>
  BitField<TType>::Bit::operator bool() const {
    return static_cast<bool>(target_ & mask_);
  }

  template <typename TType>
  bool BitField<TType>::Bit::operator~() const {
    return !static_cast<bool>(*this);
  }

  template <typename TType>
  typename BitField<TType>::Bit& BitField<TType>::Bit::operator=(const bool value) {
    if (value)
      target_ |= mask_;
    else
      target_ &= ~mask_;
    return *this;
  }

  template <typename TType>
  typename BitField<TType>::Bit& BitField<TType>::Bit::flip() {
    return *this = ~(*this);
  }
} // namespace corsika