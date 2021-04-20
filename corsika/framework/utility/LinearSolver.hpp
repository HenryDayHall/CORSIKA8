/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <vector>
#include <cmath>
#include <complex>

namespace corsika {

  std::vector<double> solve_linear_real(double a, double b, double const epsilon = 1e-12);

  std::vector<std::complex<double>> solve_linear(double a, double b,
                                                 double const epsilon = 1e-12);

} // namespace corsika

#include <corsika/detail/framework/utility/LinearSolver.inl>
