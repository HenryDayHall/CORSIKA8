/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/utl/COMBoost.h>

using namespace corsika::utl;
using namespace corsika::units::si;

COMBoost::COMBoost(HEPEnergyType eProjectile, COMBoost::MomentumVector const& pProjectile,
                   HEPMassType mTarget)
    : fRotation(Eigen::Matrix3d::Identity())
    , fCS(pProjectile.GetCoordinateSystem()) {
  // calculate matrix for rotating pProjectile to z-axis first
  auto const pProjNorm = pProjectile.norm();
  auto const a = (pProjectile / pProjNorm).GetComponents().eVector;

  if (a(0) == 0 && a(1) == 0) {
    if (a(2) < 0) {
      // if pProjectile ~ (0, 0, -1), the standard formula for the rotation matrix breaks
      // down but we can easily define a suitable rotation manually. We just need some
      // SO(3) matrix that reverses the z-axis and I like this one:

      fRotation << 1, 0, 0, 0, -1, 0, 0, 0, -1;
    }
  } else {
    Eigen::Vector3d const b{0, 0, 1};
    auto const v = a.cross(b);

    Eigen::Matrix3d vHat;
    vHat << 0, -v(2), v(1), v(2), 0, -v(0), -v(1), v(0), 0;

    fRotation += vHat + vHat * vHat / (1 + a.dot(b));
  }

  // calculate boost
  double const x = pProjNorm / (eProjectile + mTarget);

  /* Accurracy matters here, x = 1 - epsilon for ultra-relativistic boosts */
  double const coshEta = 1 / std::sqrt((1 + x) * (1 - x));
  //~ double const coshEta = 1 / std::sqrt((1-x*x));
  double const sinhEta = -x * coshEta;

  fBoost << coshEta, sinhEta, sinhEta, coshEta;

  fInverseBoost << coshEta, -sinhEta, -sinhEta, coshEta;
}

std::tuple<HEPEnergyType, corsika::geometry::QuantityVector<hepmomentum_d>>
COMBoost::toCoM(HEPEnergyType E, COMBoost::MomentumVector p) const {
  corsika::geometry::QuantityVector<hepmomentum_d> pComponents = p.GetComponents(fCS);
  Eigen::Vector3d eVecRotated = fRotation * pComponents.eVector;
  Eigen::Vector2d lab;

  lab << (E * (1 / 1_GeV)), (eVecRotated(2) * (1 / 1_GeV).magnitude());

  auto const boostedZ = fBoost * lab;
  auto const E_CoM = boostedZ(0) * 1_GeV;

  eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

  return std::make_tuple(E_CoM,
                         corsika::geometry::QuantityVector<hepmomentum_d>{eVecRotated});
}

std::tuple<HEPEnergyType, COMBoost::MomentumVector> COMBoost::fromCoM(
    HEPEnergyType E,
    corsika::geometry::QuantityVector<units::si::hepmomentum_d> pCoM) const {
  Eigen::Vector2d com;
  com << (E * (1 / 1_GeV)), (pCoM.eVector(2) * (1 / 1_GeV).magnitude());

  auto const boostedZ = fInverseBoost * com;
  auto const E_CoM = boostedZ(0) * 1_GeV;

  auto pLab = pCoM;
  pLab.eVector(2) = boostedZ(1) * (1_GeV).magnitude();
  pLab.eVector = fRotation.transpose() * pLab.eVector;

  return std::make_tuple(E_CoM, MomentumVector(fCS, pLab));
}
