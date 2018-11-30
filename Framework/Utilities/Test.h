#ifndef _utl_Test_h_
#define _utl_Test_h_

/**
   \file
   Tools to do simple testing in a readable way

   \author Lukas Nellen
   \author Darko Veberic
   \version $Id: Test.h 31925 2018-09-25 16:02:12Z darko $
   \date 08 Feb 2004

   \ingroup testing
*/

#include <cmath>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <utl/Triple.h>


namespace utl {

  //! Predicate used in STL for searching for whitespace
  struct IsSpace {
    bool operator()(const char x) const
    { return x == ' ' || x == '\r' || x == '\n' || x == '\t'; }
  };


  /// Predicate for equality
  class Equal {
  public:
    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return lhs == rhs; }

    static const char* Name() { return "equal"; }
  };


  /// Predicate for less
  class Less {
  public:
    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return lhs < rhs; }

    static const char* Name() { return "less"; }
  };


  /// Predicate for less or equal
  class LessOrEqual {
  public:
    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return lhs <= rhs; }

    static const char* Name() { return "less or equal"; }
  };


  /// Predicate for greater
  class Greater {
  public:
    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return lhs > rhs; }

    static const char* Name() { return "greater"; }
  };


  /// Predicate for greater or equal
  class GreaterOrEqual {
  public:
    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return lhs >= rhs; }

    static const char* Name() { return "greater or equal"; }
  };

  /// Predicate for approximate equality (for floating point)
  /** The default precision is 1e-6, but it can be changed at
      construction time.
   */
  class CloseTo {
  public:
    CloseTo(const double eps = 1e-6) : fEpsilon(eps) { }

    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return IsCloseTo(lhs, rhs); }

    boost::format Name() const
    { return boost::format("close (@%g) to") % fEpsilon; }

  protected:
    template<typename T>
    bool IsCloseAbs(const T& lhs, const T& rhs) const
    { return std::abs(double(lhs) - double(rhs)) < fEpsilon; }

    bool IsCloseAbs(const utl::Triple& lhs, const utl::Triple& rhs) const
    { return std::sqrt(TupleDist2(lhs, rhs)) < fEpsilon; }

    template<typename T>
    bool IsCloseRel(const T& lhs, const T& rhs) const
    { return 2*std::abs(double(lhs) - double(rhs)) / (std::abs(double(lhs)) + std::abs(double(rhs))) < fEpsilon; }

    bool
    IsCloseRel(const utl::Triple& lhs, const utl::Triple& rhs)
      const
    {
      return (2*sqrt(TupleDist2(lhs, rhs)) /
        (sqrt(TupleDist2(lhs, utl::Triple(0,0,0))) +
         sqrt(TupleDist2(rhs, utl::Triple(0,0,0))))) < fEpsilon;
    }

    template<typename T>
    bool
    IsCloseTo(const T& lhs, const T& rhs)
      const
    {
      if (IsCloseAbs(lhs, rhs))
        return true;
      else
        return IsCloseRel(lhs, rhs);
    }

    // tuple distance
    template<typename Head, typename Tail>
    static
    double
    TupleDist2(const boost::tuples::cons<Head, Tail>& lhs,
               const boost::tuples::cons<Head, Tail>& rhs)
    {
      const double t = lhs.get_head() - rhs.get_head();
      return t*t + TupleDist2(lhs.get_tail(), rhs.get_tail());
    }

    static double TupleDist2(const boost::tuples::null_type& /*lhs*/,
                             const boost::tuples::null_type& /*rhs*/)
    { return 0; }

    double fEpsilon;
  };


  class CloseAbs : public CloseTo {
  public:
    CloseAbs(const double eps = 1e-6) : CloseTo(eps) { }

    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return IsCloseAbs(lhs, rhs); }

    boost::format Name() const
    { return boost::format("absolutely close (@%g) to") % fEpsilon; }
  };


  class CloseRel : public CloseTo {
  public:
    CloseRel(const double eps = 1e-6) : CloseTo(eps) { }

    template<typename T>
    bool operator()(const T& lhs, const T& rhs) const
    { return IsCloseRel(lhs, rhs); }

    boost::format Name() const
    { return boost::format("relatively close (@%g) to") % fEpsilon; }
  };


  template<typename Predicate>
  class Not : public Predicate {
  public:
    Not() : Predicate() { }
    
    Not(const double eps) : Predicate(eps) { }
    
    template<typename T>
    bool operator()(const T& x) const
    { return !Predicate::operator()(x); }

    template<typename T, typename U>
    bool operator()(const T& x, const U& y) const
    { return !Predicate::operator()(x, y); }

    template<typename T, typename U, typename W>
    bool operator()(const T& x, const U& y, const W& z) const
    { return !Predicate::operator()(x, y, z); }

    static boost::format Name()
    { return boost::format("not-%s") % Predicate().Name(); }
  };


  inline
  utl::Triple
  Diff(const utl::Triple& lhs, const utl::Triple& rhs)
  {
    return utl::Triple(lhs.get<0>() - rhs.get<0>(),
                       lhs.get<1>() - rhs.get<1>(),
                       lhs.get<2>() - rhs.get<2>());
  }


  /// Test condition by evaluating a predicate
  /** If the predicate evaluates to false, we print a failure message
      with the values of the left- and right-hand side and the name of
      the predicate. This information is normally not available when
      using the CPPUNIT_ASSERT macro.
   */
  template<class Predicate, typename T>
  inline bool Test(const Predicate& pred, const T& lhs, const T& rhs)
  { return pred(lhs, rhs); }


  /// Main test function
  template<class Predicate, typename T>
  inline bool Test(const T& lhs, const T& rhs)
  { return Test(Predicate(), lhs, rhs); }


  /// Test function for predicates that take an option
  template<class Predicate, typename T, typename U>
  inline bool Test(const T& lhs, const T& rhs, const U& eps)
  { return Test(Predicate(eps), lhs, rhs); }

}


#endif
