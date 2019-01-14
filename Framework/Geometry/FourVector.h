#ifndef _include_corsika_framework_geometry_fourvector_h_
#define _include_corsika_framework_geometry_fourvector_h_

#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

#include <iostream>
#include <type_traits>

namespace corsika::geometry {

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

template <typename TimeType, typename SpaceVec> class FourVector {

public:
  using SpaceType = typename std::decay<SpaceVec>::type::Quantity;

  /// check the types and the physical units here:
  static_assert(
      std::is_same<typename std::decay<TimeType>::type, SpaceType>::value ||
          std::is_same<typename std::decay<TimeType>::type,
                       decltype(std::declval<SpaceType>() /
                                corsika::units::si::meter *
                                corsika::units::si::second)>::value,
      "Units of time-like and space-like coordinates must either be idential "
      "(e.g. GeV) or [E/c]=[p]");

public:
  /*
  template <typename TT, typename SS>
  FourVector(TT && eT, SS && eS)
    : fTimeLike(std::forward<TT>(eT)), fSpaceLike(std::forward<SS>(eS)) {
    std::cout << "FourVector&&\n"; }
  */

  FourVector(const TimeType &eT, const SpaceVec &eS)
      : fTimeLike(eT), fSpaceLike(eS) {
    // std::cout << "FourVector const &\n";
  }

  /*
 FourVector(TimeType &eT, SpaceVec &eS)
      : fTimeLike(eT), fSpaceLike(eS) {
    std::cout << "FourVector &\n"; }
  */

  TimeType GetTime() { return fTimeLike; }

  auto GetNormSqr() const {
    return GetTimeSquared() - fSpaceLike.squaredNorm();
  }

  SpaceType GetNorm() const { return sqrt(abs(GetNormSqr())); }

  bool IsTimelike() const {
    return GetTimeSquared() < fSpaceLike.squaredNorm();
  } // Norm2 < 0

  bool IsSpacelike() const {
    return GetTimeSquared() > fSpaceLike.squaredNorm();
  } // Norm2 > 0

  bool IsPhotonlike() const {
    return GetTimeSquared() == fSpaceLike.squaredNorm();
  } // // Norm2 == 0

  FourVector &operator+=(const FourVector &b) {
    fTimeLike += b.fTimeLike;
    fSpaceLike += b.fSpaceLike;
    return *this;
  }

  FourVector &operator-=(const FourVector &b) {
    fTimeLike -= b.fTimeLike;
    fSpaceLike -= b.fSpaceLike;
    return *this;
  }

  FourVector &operator*=(const double b) {
    fTimeLike *= b;
    fSpaceLike *= b;
    return *this;
  }

  FourVector &operator/=(const double b) {
    fTimeLike /= b;
    fSpaceLike.GetComponents() /= b; // TODO: WHY IS THIS??????
    return *this;
  }

  FourVector &operator/(const double b) {
    *this /= b;
    return *this;
  }

  /**
     Note that the product between two 4-vectors assumes that you use
     the same "c" convention for both. Only the LHS vector is checked
     for this. You cannot mix different conventions due to
     unit-checking.
   */
  SpaceType operator*(const FourVector &b) {
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

private:
  /**
     This function is automatically compiled to use of ignore the
     extra factor of "c" for the time-like quantity
   */
  auto GetTimeSquared() const {
    if constexpr (std::is_same<typename std::decay<TimeType>::type,
                               decltype(std::declval<SpaceType>() /
                                        corsika::units::si::meter *
                                        corsika::units::si::second)>::value)
      return fTimeLike * fTimeLike *
             (corsika::units::constants::c * corsika::units::constants::c);
    else
      return fTimeLike * fTimeLike;
  }

protected:
  /// the data members
  TimeType fTimeLike;
  SpaceVec fSpaceLike;

  
  /// the friends: math operators
  template <typename T, typename U>
  friend FourVector<T, U> operator+(const FourVector<T, U> &,
                                    const FourVector<T, U> &);

  template <typename T, typename U>
  friend FourVector<T, U> operator-(const FourVector<T, U> &,
                                    const FourVector<T, U> &);

  template <typename T, typename U>
  friend FourVector<T, U> operator*(const FourVector<T, U> &, const double);

    template <typename T, typename U>
  friend FourVector<T, U> operator/(const FourVector<T, U> &, const double);

};

/*
//template<typename T, typename U> FourVector(T& t, U& u) ->
FourVector<decltype(t), decltype(u)>; template<typename T, typename U>
FourVector(const T& t, const U& u) -> FourVector<const typename
std::decay<T>::type, const typename std::decay<U>::type>; template<typename T,
typename U> FourVector(T&& t, U&& u) -> FourVector<typename std::decay<T>::type,
typename std::decay<U>::type>;
 // template<typename T, typename U> FourVector(T&& t, U&& u) ->
FourVector<decltype(t), decltype(u)>;
 */

/** 
    The math operator+
 */
template <typename TimeType, typename SpaceVec>
inline FourVector<TimeType, SpaceVec>
operator+(const FourVector<TimeType, SpaceVec> &a,
          const FourVector<TimeType, SpaceVec> &b) {
  return FourVector<TimeType, SpaceVec>(a.fTimeLike + b.fTimeLike,
                                        a.fSpaceLike + b.fSpaceLike);
}

 /** 
    The math operator-
 */
template <typename TimeType, typename SpaceVec>
inline FourVector<TimeType, SpaceVec>
operator-(const FourVector<TimeType, SpaceVec> &a,
          const FourVector<TimeType, SpaceVec> &b) {
  return FourVector<TimeType, SpaceVec>(a.fTimeLike - b.fTimeLike,
                                        a.fSpaceLike - b.fSpaceLike);
}

 /** 
    The math operator*
 */
template <typename TimeType, typename SpaceVec>
inline FourVector<TimeType, SpaceVec>
operator*(const FourVector<TimeType, SpaceVec> &a, const double b) {
  return FourVector<TimeType, SpaceVec>(a.fTimeLike * b, a.fSpaceLike * b);
}

/** 
    The math operator/
 */
 template <typename TimeType, typename SpaceVec>
inline FourVector<TimeType, SpaceVec>
operator/(const FourVector<TimeType, SpaceVec> &a, const double b) {
  return FourVector<TimeType, SpaceVec>(a.fTimeLike / b, a.fSpaceLike / b);
}

} // namespace corsika::geometry

#endif
