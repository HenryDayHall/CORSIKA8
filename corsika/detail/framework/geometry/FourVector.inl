/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
  inline TTimeType FourVector<TTimeType, TSpaceVecType>::getTimeLikeComponent() const {
    return timeLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline TSpaceVecType& FourVector<TTimeType, TSpaceVecType>::getSpaceLikeComponents() {
    return spaceLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline TSpaceVecType const&
  FourVector<TTimeType, TSpaceVecType>::getSpaceLikeComponents() const {
    return spaceLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline typename FourVector<TTimeType, TSpaceVecType>::norm_square_type
  FourVector<TTimeType, TSpaceVecType>::getNormSqr() const {
    return getTimeSquared() - spaceLike_.getSquaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline typename FourVector<TTimeType, TSpaceVecType>::norm_type
  FourVector<TTimeType, TSpaceVecType>::getNorm() const {

    return sqrt(abs(getNormSqr()));
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline bool FourVector<TTimeType, TSpaceVecType>::isTimelike() const {
    return getTimeSquared() < spaceLike_.getSquaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline bool FourVector<TTimeType, TSpaceVecType>::isSpacelike() const {
    return getTimeSquared() > spaceLike_.getSquaredNorm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline FourVector<TTimeType, TSpaceVecType>& FourVector<TTimeType, TSpaceVecType>::
  operator+=(FourVector const& b) {
    timeLike_ += b.timeLike_;
    spaceLike_ += b.spaceLike_;

    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline FourVector<TTimeType, TSpaceVecType>&
  FourVector<TTimeType, TSpaceVecType>::operator-=(FourVector const& b) {
    timeLike_ -= b.timeLike_;
    spaceLike_ -= b.spaceLike_;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline FourVector<TTimeType, TSpaceVecType>&
  FourVector<TTimeType, TSpaceVecType>::operator*=(double const b) {
    timeLike_ *= b;
    spaceLike_ *= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline FourVector<TTimeType, TSpaceVecType>&
  FourVector<TTimeType, TSpaceVecType>::operator/=(double const b) {
    timeLike_ /= b;
    spaceLike_.getComponents() /= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline FourVector<TTimeType, TSpaceVecType>&
  FourVector<TTimeType, TSpaceVecType>::operator/(double const b) {
    *this /= b;
    return *this;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline typename FourVector<TTimeType, TSpaceVecType>::norm_type
  FourVector<TTimeType, TSpaceVecType>::operator*(FourVector const& b) {
    if constexpr (std::is_same<time_type, decltype(std::declval<space_type>() / meter *
                                                   second)>::value)
      return timeLike_ * b.timeLike_ * constants::cSquared - spaceLike_.norm();
    else
      return timeLike_ * timeLike_ - spaceLike_.norm();
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline typename FourVector<TTimeType, TSpaceVecType>::norm_square_type
  FourVector<TTimeType, TSpaceVecType>::getTimeSquared() const {
    if constexpr (std::is_same<time_type, decltype(std::declval<space_type>() / meter *
                                                   second)>::value)
      return timeLike_ * timeLike_ * constants::cSquared;
    else
      return timeLike_ * timeLike_;
  }

  template <typename TTimeType, typename TSpaceVecType>
  inline std::ostream& operator<<(
      std::ostream& os, corsika::FourVector<TTimeType, TSpaceVecType> const qv) {

    os << '(' << qv.timeLike_ << ", " << qv.spaceLike_ << ") ";
    return os;
  }

} // namespace corsika
