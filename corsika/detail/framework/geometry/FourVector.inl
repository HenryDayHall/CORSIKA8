/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <type_traits>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  template <typename TTimeType, typename TSpaceVecType>
  TTimeType FourVector<TTimeType, TSpaceVecType>::getTimeLikeComponent() const {
    return timeLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  TSpaceVecType& FourVector<TTimeType, TSpaceVecType>::spaceLikeComponents() {
    return spaceLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  const TSpaceVecType& FourVector<TTimeType, TSpaceVecType>::getSpaceLikeComponents()
      const {
    return spaceLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  typename FourVector<TTimeType, TSpaceVecType>::norm_square_type
  FourVector<TTimeType, TSpaceVecType>::getNormSqr() const {
    return getTimeSquared() - spaceLike_.squaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  typename FourVector<TTimeType, TSpaceVecType>::norm_type
  FourVector<TTimeType, TSpaceVecType>::getNorm() const {

    return sqrt(abs(getNormSqr()));
  }

  template <typename TTimeType, typename TSpaceVecType>
  bool FourVector<TTimeType, TSpaceVecType>::isTimelike() const {
    return getTimeSquared() < spaceLike_.squaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  bool FourVector<TTimeType, TSpaceVecType>::isSpacelike() const {
    return getTimeSquared() > spaceLike_.squaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::operator+=(
      const FourVector& b) {
    timeLike_ += b.timeLike_;
    spaceLike_ += b.spaceLike_;

    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::operator-=(
      const FourVector& b) {
    timeLike_ -= b.timeLike_;
    spaceLike_ -= b.spaceLike_;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::operator*=(
      const double b) {
    timeLike_ *= b;
    spaceLike_ *= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::operator/=(
      const double b) {
    timeLike_ /= b;
    spaceLike_.GetComponents() /= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::operator/(
      const double b) {
    *this /= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  typename FourVector<TTimeType, TSpaceVecType>::norm_type
      FourVector<TTimeType, TSpaceVecType>::operator*(const FourVector& b) {
    if constexpr (std::is_same<time_type, decltype(std::declval<space_type>() / meter *
                                                   second)>::value)
      return timeLike_ * b.timeLike_ * constants::cSquared - spaceLike_.norm();
    else
      return timeLike_ * timeLike_ - spaceLike_.norm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  typename FourVector<TTimeType, TSpaceVecType>::norm_square_type
  FourVector<TTimeType, TSpaceVecType>::getTimeSquared() const {
    if constexpr (std::is_same<time_type, decltype(std::declval<space_type>() / meter *
                                                   second)>::value)
      return timeLike_ * timeLike_ * constants::cSquared;
    else
      return timeLike_ * timeLike_;
  }

} // namespace corsika
