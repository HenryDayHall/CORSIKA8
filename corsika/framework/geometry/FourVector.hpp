/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <iostream>
#include <type_traits>

namespace corsika {

  /**
     FourVector supports "full" units, e.g. E in [GeV/c] and p in [GeV],
     or also t in [s] and r in [m], etc.

     However, for HEP applications it is also possible to use E and p
     both in [GeV].

     The FourVector can return NormSqr and Norm, whereas Norm is
     sqrt(abs(NormSqr)). The physical units are always calculated and
     returned properly.

     FourVector can also return if it is TimeLike, SpaceLike or PhotonLike.

     When a FourVector is initialized with a lvalue reference, this is
     also used for the internal storage, which should lead to complete
     disappearance of the FourVector class during optimization.
   */

  template <typename TimeType, typename SpaceVecType>
  class FourVector {

  public:
    using SpaceType = typename std::decay<SpaceVecType>::type::Quantity;

    //! check the types and the physical units here:
    static_assert(
        std::is_same<typename std::decay<TimeType>::type, SpaceType>::value ||
            std::is_same<typename std::decay<TimeType>::type,
                         decltype(std::declval<SpaceType>() / corsika::units::si::meter *
                                  corsika::units::si::second)>::value,
        "Units of time-like and space-like coordinates must either be idential "
        "(e.g. GeV) or [E/c]=[p]");

  public:
    FourVector() = default;

    FourVector(const TimeType& eT, const SpaceVecType& eS)
        : fTimeLike(eT)
        , fSpaceLike(eS) {}

    /*
     * FIXME: These Getters are mis-leading and does not favor
     * locality. Adhere to Getter/Setter
     */
    /**
     * @brief
     *
     * @return fTimeLike
     */
    TimeType GetTimeLikeComponent() const;

    /**
     * @brief
     *
     * @return fSpaceLike
     */
    SpaceVecType& GetSpaceLikeComponents();

    /**
     * @brief
     *
     * @return fSpaceLike;
     */
    const SpaceVecType& GetSpaceLikeComponents() const;

    /**
     * @brief
     *
     * @return
     */
    auto GetNormSqr() const;

    /**
     * @brief
     *
     * @return
     */
    SpaceType GetNorm() const;

    /*
     * FIXME: a better alternative would be to define an enumeration
     * enum { SpaceLike =-1, TimeLike, LightLike } V4R_Category;
     * and a method called  V4R_Category GetCategory() const;
     */
    /**
     * @brief
     *
     * @return
     */
    bool IsTimelike() const;

    /**
     * @brief
     *
     * @return
     */
    bool IsSpacelike() const;

    FourVector& operator+=(const FourVector& b);

    FourVector& operator-=(const FourVector& b);

    FourVector& operator*=(const double b);

    FourVector& operator/=(const double b);

    FourVector& operator/(const double b);

    /**
       Note that the product between two 4-vectors assumes that you use
       the same "c" convention for both. Only the LHS vector is checked
       for this. You cannot mix different conventions due to
       unit-checking.
     */
    SpaceType operator*(const FourVector& b);

  protected:
    //! the data members
    TimeType fTimeLike;
    SpaceVecType fSpaceLike;

    //! the friends: math operators
    template <typename T, typename U>
    friend FourVector<typename std::decay<T>::type, typename std::decay<U>::type>
    operator+(const FourVector<T, U>&, const FourVector<T, U>&);

    template <typename T, typename U>
    friend FourVector<typename std::decay<T>::type, typename std::decay<U>::type>
    operator-(const FourVector<T, U>&, const FourVector<T, U>&);

    template <typename T, typename U>
    friend FourVector<typename std::decay<T>::type, typename std::decay<U>::type>
    operator*(const FourVector<T, U>&, const double);

    template <typename T, typename U>
    friend FourVector<typename std::decay<T>::type, typename std::decay<U>::type>
    operator/(const FourVector<T, U>&, const double);

  private:
    /**
       This function is automatically compiled to use of ignore the
       extra factor of "c" for the time-like quantity
     */
    auto GetTimeSquared() const;
  };

  /**
      The math operator+
   */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator+(const FourVector<TimeType, SpaceVecType>& a,
            const FourVector<TimeType, SpaceVecType>& b);

  /**
     The math operator-
  */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator-(const FourVector<TimeType, SpaceVecType>& a,
            const FourVector<TimeType, SpaceVecType>& b);

  /**
     The math operator*
     FIXME: Add overload to deal with multiplication by a scalar and 3-vectors
  */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator*(const FourVector<TimeType, SpaceVecType>& a, const double b);

  /**
      The math operator/
   */
  template <typename TimeType, typename SpaceVecType>
  inline FourVector<typename std::decay<TimeType>::type,
                    typename std::decay<SpaceVecType>::type>
  operator/(const FourVector<TimeType, SpaceVecType>& a, const double b);

} // namespace corsika

#include <corsika/detail/framework/geometry/FourVector.inl>
