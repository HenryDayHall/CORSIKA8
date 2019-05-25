/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <corsika/utl/sgn.h>

using namespace corsika::utl;
using namespace corsika::units::si;
using namespace corsika::geometry;

COMBoost::COMBoost(FourVector<HEPEnergyType, Vector<hepmomentum_d>> const& Pprojectile,
                   const HEPMassType massTarget)
    : fCS(Pprojectile.GetSpaceLikeComponents().GetCoordinateSystem()) {
  auto const pProjectile = Pprojectile.GetSpaceLikeComponents();
  auto const pProjNorm = pProjectile.norm();
  auto const a = (pProjectile / pProjNorm).GetComponents().eVector;
  auto const a1 = a(0), a2 = a(1);

  auto const s = sgn(a(2));
  auto const c = 1 / (1 + s * a(2));

  Eigen::Matrix3d A, B;

  if (s > 0) {
    A << 1, 0, -a1,                     // comment to prevent clang-format
        0, 1, -a2,                      // .
        a1, a2, 1;                      // .
    B << -a1 * a1 * c, -a1 * a2 * c, 0, // .
        -a1 * a2 * c, -a2 * a2 * c, 0,  // .
        0, 0, -(a1 * a1 + a2 * a2) * c; // .

  } else {
    A << 1, 0, a1,                      // comment to prevent clang-format
        0, -1, -a2,                     // .
        a1, a2, -1;                     // .
    B << -a1 * a1 * c, -a1 * a2 * c, 0, // .
        +a1 * a2 * c, +a2 * a2 * c, 0,  // .
        0, 0, (a1 * a1 + a2 * a2) * c;  // .
  }

  fRotation = A + B;

  // calculate boost
  double const beta = pProjNorm / (Pprojectile.GetTimeLikeComponent() + massTarget);

  /* Accurracy matters here, beta = 1 - epsilon for ultra-relativistic boosts */
  double const coshEta = 1 / std::sqrt((1 + beta) * (1 - beta));
  //~ double const coshEta = 1 / std::sqrt((1-beta*beta));
  double const sinhEta = -beta * coshEta;

  std::cout << "COMBoost (1-beta)=" << 1 - beta << " gamma=" << coshEta << std::endl;
  std::cout << "  det = " << fRotation.determinant() - 1 << std::endl;

  fBoost << coshEta, sinhEta, sinhEta, coshEta;

  fInverseBoost << coshEta, -sinhEta, -sinhEta, coshEta;
}

/*
  Here we instantiate all physically meaningful versions of COMBoost
 */
