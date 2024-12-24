/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>

#include <tuple>

namespace corsika {

  /**
   * Interpolates profiles around maximum.
   *
   * The maximum of profiles can be robustly estimated from the three maximal points by
   * quadratic interpolation. This code is copied from CONEX and is awaiting full adaption
   * into CORSIKA8. This is a temporary solution (just copied).
   */

  class FindXmax {

    static bool invert3by3(double A[3][3]) {
      const double kSmall = 1e-80;

      double determinant = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) -
                           A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
                           A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);

      double absDet = fabs(determinant);

      if (absDet < kSmall) {
        CORSIKA_LOG_WARN("invert3by3: Error-matrix singular (absDet={})", absDet);
        return false;
      }

      double B[3][3];

      B[0][0] = A[1][1] * A[2][2] - A[1][2] * A[2][1];
      B[1][0] = A[1][2] * A[2][0] - A[2][2] * A[1][0];
      B[2][0] = A[1][0] * A[2][1] - A[1][1] * A[2][0];

      B[0][1] = A[0][2] * A[2][1] - A[2][2] * A[0][1];
      B[1][1] = A[0][0] * A[2][2] - A[2][0] * A[0][2];
      B[2][1] = A[0][1] * A[2][0] - A[0][0] * A[2][1];

      B[0][2] = A[0][1] * A[1][2] - A[1][1] * A[0][2];
      B[1][2] = A[0][2] * A[1][0] - A[1][2] * A[0][0];
      B[2][2] = A[0][0] * A[1][1] - A[0][1] * A[1][0];

      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) A[i][j] = B[i][j] / determinant;

      return true;
    }

    /****************************************************
     *
     * solves linear system
     *
     *   / y[0] \   / A[0][0] A[0][1] A[0][2] \   / x[0] \
     *   | y[1] | = | A[1][0] A[1][1] A[1][2] | * | x[1] |
     *   \ y[2] /   \ A[2][0] A[2][1] A[2][2] /   \ x[2] /
     *
     *
     * Input:  y[3] and A[3][3]
     * Output: returns true when succeded (i.e. A is not singular)
     *         A is overwritten with its inverse
     *
     * M.Unger 12/1/05
     *
     ****************************************************/
    static bool solve3by3(double y[3], double A[3][3], double x[3]) {
      if (invert3by3(A)) {

        for (int i = 0; i < 3; i++)
          x[i] = A[i][0] * y[0] + A[i][1] * y[1] + A[i][2] * y[2];

        return true;

      } else
        return false;
    }

  public:
    static std::tuple<double, double> interpolateProfile(double x1, double x2, double x3,
                                                         double y1, double y2,
                                                         double y3) {

      // quadratic "fit" around maximum to get dEdXmax and Xmax
      double x[3] = {x1, x2, x3};
      double y[3] = {y1, y2, y3};

      double A[3][3];
      A[0][0] = x[0] * x[0];
      A[0][1] = x[0];
      A[0][2] = 1.;
      A[1][0] = x[1] * x[1];
      A[1][1] = x[1];
      A[1][2] = 1.;
      A[2][0] = x[2] * x[2];
      A[2][1] = x[2];
      A[2][2] = 1.;

      double a[3];

      solve3by3(y, A, a);

      if (a[0] < 0.) {
        double const Xmax = -a[1] / (2. * a[0]);
        return std::make_tuple(Xmax, a[0] * Xmax * Xmax + a[1] * Xmax + a[2]);
      }
      return std::make_tuple(0, 0);
    }
  };
} // namespace corsika