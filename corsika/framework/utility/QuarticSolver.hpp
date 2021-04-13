/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <complex>

/**
 * @file QuarticSolver.hpp
 * @ingroup Utilities
 * @{
 * \todo convert to class
 */

namespace corsika::quartic_solver {

  const double PI = 3.141592653589793238463L;
  const double M_2PI = 2 * PI;
  const double eps = 1e-12;

  typedef std::complex<double> DComplex;

  //---------------------------------------------------------------------------
  // useful for testing
  inline DComplex polinom_2(DComplex x, double a, double b) {
    // Horner's scheme for x*x + a*x + b
    return x * (x + a) + b;
  }

  //---------------------------------------------------------------------------
  // useful for testing
  inline DComplex polinom_3(DComplex x, double a, double b, double c) {
    // Horner's scheme for x*x*x + a*x*x + b*x + c;
    return x * (x * (x + a) + b) + c;
  }

  //---------------------------------------------------------------------------
  // useful for testing
  inline DComplex polinom_4(DComplex x, double a, double b, double c, double d) {
    // Horner's scheme for x*x*x*x + a*x*x*x + b*x*x + c*x + d;
    return x * (x * (x * (x + a) + b) + c) + d;
  }

  //---------------------------------------------------------------------------
  // x - array of size 3
  // In case 3 real roots: => x[0], x[1], x[2], return 3
  //         2 real roots: x[0], x[1],          return 2
  //         1 real root : x[0], x[1] ± i*x[2], return 1
  unsigned int solveP3(double* x, double a, double b, double c);

  //---------------------------------------------------------------------------
  // solve quartic equation x^4 + a*x^3 + b*x^2 + c*x + d
  // Attention - this function returns dynamically allocated array. It has to be released
  // afterwards.
  DComplex* solve_quartic(double a, double b, double c, double d);

  //! @}

} // namespace corsika::quartic_solver

#include <corsika/detail/framework/utility/QuarticSolver.inl>
