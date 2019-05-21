#ifndef _utilities_sgn_h
#define _utilities_sgn_h

namespace corsika::utl {

  //! sign function without branches
  template <typename T>
  static int sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

} // namespace corsika::utl

#endif
