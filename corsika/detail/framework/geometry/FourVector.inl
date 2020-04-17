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

#include <iostream>
#include <type_traits>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>



namespace corsika {


	template <typename TimeType, typename SpaceVecType>
    TimeType FourVector<TimeType, SpaceVecType>::GetTimeLikeComponent() const
    {
    	return fTimeLike;
    }

	template <typename TimeType, typename SpaceVecType>
    SpaceVecType& FourVector<TimeType, SpaceVecType>::GetSpaceLikeComponents()
	{
		return fSpaceLike;
	}


	template <typename TimeType, typename SpaceVecType>
    const SpaceVecType& FourVector<TimeType, SpaceVecType>::GetSpaceLikeComponents() const
	{
		return fSpaceLike;
	}

	template <typename TimeType, typename SpaceVecType>
    auto FourVector<TimeType, SpaceVecType>::GetNormSqr() const
	{
		return GetTimeSquared() - fSpaceLike.squaredNorm();
	}

	template <typename TimeType, typename SpaceVecType>
	typename FourVector<TimeType, SpaceVecType>::SpaceType
	FourVector<TimeType, SpaceVecType>::GetNorm() const
	{

		return sqrt(abs(GetNormSqr()));
	}

	template <typename TimeType, typename SpaceVecType>
    bool FourVector<TimeType, SpaceVecType>::IsTimelike() const
	{
      return GetTimeSquared() < fSpaceLike.squaredNorm();
    }

	template <typename TimeType, typename SpaceVecType>
    bool FourVector<TimeType, SpaceVecType>::IsSpacelike() const {
      return GetTimeSquared() > fSpaceLike.squaredNorm();
    }

	template <typename TimeType, typename SpaceVecType>
    FourVector<TimeType, SpaceVecType>&
	FourVector<TimeType, SpaceVecType>::operator+=(const FourVector& b)
	{
      fTimeLike += b.fTimeLike;
      fSpaceLike += b.fSpaceLike;

      return *this;
    }

	template <typename TimeType, typename SpaceVecType>
    FourVector<TimeType, SpaceVecType>&
	FourVector<TimeType, SpaceVecType>::operator-=(const FourVector& b)
	{
      fTimeLike -= b.fTimeLike;
      fSpaceLike -= b.fSpaceLike;
      return *this;
    }

	template <typename TimeType, typename SpaceVecType>
    FourVector<TimeType, SpaceVecType>&
	FourVector<TimeType, SpaceVecType>::operator*=(const double b)
	{
      fTimeLike *= b;
      fSpaceLike *= b;
      return *this;
    }

	template <typename TimeType, typename SpaceVecType>
    FourVector<TimeType, SpaceVecType>&
	FourVector<TimeType, SpaceVecType>::operator/=(const double b)
	{
      fTimeLike /= b;
      fSpaceLike.GetComponents() /= b; // TODO: WHY IS THIS??????
      return *this;
    }

	template <typename TimeType, typename SpaceVecType>
    FourVector<TimeType, SpaceVecType>&
	FourVector<TimeType, SpaceVecType>::operator/(const double b)
	{
      *this /= b;
      return *this;
    }

	template <typename TimeType, typename SpaceVecType>
	typename FourVector<TimeType, SpaceVecType>::SpaceType
	FourVector<TimeType, SpaceVecType>::operator*(const FourVector& b)
	{
      if constexpr (std::is_same<typename std::decay<TimeType>::type,
                                 decltype(std::declval<SpaceType>() /
                                          corsika::units::si::meter *
                                          corsika::units::si::second)>::value)
        return fTimeLike * b.fTimeLike *
                   (corsika::units::constants::c * corsika::units::constants::c) -
               fSpaceLike.norm();
      else
        return fTimeLike * fTimeLike - fSpaceLike.norm();
    }

	template <typename TimeType, typename SpaceVecType>
    auto FourVector<TimeType, SpaceVecType>::GetTimeSquared() const
	{
      if constexpr (std::is_same<typename std::decay<TimeType>::type,
                                 decltype(std::declval<SpaceType>() /
                                          corsika::units::si::meter *
                                          corsika::units::si::second)>::value)
        return fTimeLike * fTimeLike *
               (corsika::units::constants::c * corsika::units::constants::c);
      else
        return fTimeLike * fTimeLike;
    }

  /**
      The math operator+
   */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type, typename std::decay<SpaceVecType>::type>
  operator+(const FourVector<TimeType, SpaceVecType>& a, const FourVector<TimeType, SpaceVecType>& b)
  {
    return FourVector<typename std::decay<TimeType>::type,
    		          typename std::decay<SpaceVecType>::type>(a.fTimeLike + b.fTimeLike, a.fSpaceLike + b.fSpaceLike);
  }

  /**
     The math operator-
  */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator-(const FourVector<TimeType, SpaceVecType>& a, const FourVector<TimeType, SpaceVecType>& b)
  {
    return FourVector<typename std::decay<TimeType>::type,
                      typename std::decay<SpaceVecType>::type>(
        a.fTimeLike - b.fTimeLike, a.fSpaceLike - b.fSpaceLike);
  }

  /**
     The math operator*
  */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator*(const FourVector<TimeType, SpaceVecType>& a, const double b)
  {
    return FourVector<typename std::decay<TimeType>::type,
                      typename std::decay<SpaceVecType>::type>(a.fTimeLike * b,
                                                               a.fSpaceLike * b);
  }

  /**
      The math operator/
   */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator/(const FourVector<TimeType, SpaceVecType>& a, const double b) {
    return FourVector<typename std::decay<TimeType>::type,
                      typename std::decay<SpaceVecType>::type>(a.fTimeLike / b,
                                                               a.fSpaceLike / b);
  }

} // namespace corsika
