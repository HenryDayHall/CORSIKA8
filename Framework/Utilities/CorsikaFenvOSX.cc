/**
 * Import public domain code
 *
 * Provide portable or fallback versions of feenableexcept() and fedisableexcept()
 * Exist by default in glibc since version 2.2, but not in the standard
 * fenv.h / cfenv headers for C 99 or C++ 11
 *
 * \author Lukas Nellen
 * \date 14 Jan 2019
 *
 */

#include <corsika/utl/CorsikaFenv.h>
#include <cfenv>

// Implementation for OS X on intel X64_86
// code from
// https://stackoverflow.com/questions/37819235/how-do-you-enable-floating-point-exceptions-for-clang-in-os-x
// based on
// http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

extern "C" {

int feenableexcept(int excepts) {
  static fenv_t fenv;
  int new_excepts = excepts & FE_ALL_EXCEPT;
  // previous masks
  int old_excepts;

  if (fegetenv(&fenv)) { return -1; }
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // unmask
  fenv.__control &= ~new_excepts;
  fenv.__mxcsr &= ~(new_excepts << 7);

  return fesetenv(&fenv) ? -1 : old_excepts;
}

int fedisableexcept(int excepts) {
  static fenv_t fenv;
  int new_excepts = excepts & FE_ALL_EXCEPT;
  // all previous masks
  int old_excepts;

  if (fegetenv(&fenv)) { return -1; }
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // mask
  fenv.__control |= new_excepts;
  fenv.__mxcsr |= new_excepts << 7;

  return fesetenv(&fenv) ? -1 : old_excepts;
}
}
