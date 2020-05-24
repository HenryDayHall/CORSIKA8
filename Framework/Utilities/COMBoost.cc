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

#include <cmath>

using namespace corsika::utl;
using namespace corsika::units::si;
using namespace corsika::geometry;

COMBoost::COMBoost(FourVector<HEPEnergyType, Vector<hepmomentum_d>> const& Pprojectile,
                   const HEPMassType massTarget)
    : originalCS_{Pprojectile.GetSpaceLikeComponents().GetCoordinateSystem()}
    , rotatedCS_{originalCS_.RotateToZ(Pprojectile.GetSpaceLikeComponents())} {
  auto const pProjectile = Pprojectile.GetSpaceLikeComponents();
  auto const pProjNormSquared = pProjectile.squaredNorm();
  auto const pProjNorm = sqrt(pProjNormSquared);

  auto const eProjectile = Pprojectile.GetTimeLikeComponent();
  auto const massProjectileSquared = eProjectile * eProjectile - pProjNormSquared;
  auto const s =
      massTarget * massTarget + massProjectileSquared + 2 * eProjectile * massTarget;

  auto const sqrtS = sqrt(s);
  auto const sinhEta = -pProjNorm / sqrtS;
  auto const coshEta = sqrt(1 + pProjNormSquared / s);

  setBoost(coshEta, sinhEta);

  std::cout << "COMBoost (1-beta)=" << 1 - sinhEta / coshEta << " gamma=" << coshEta
            << std::endl;
  std::cout << "  det = " << boost_.determinant() - 1 << std::endl;
}

COMBoost::COMBoost(geometry::Vector<units::si::hepmomentum_d> const& momentum,
                   units::si::HEPEnergyType mass)
    : originalCS_{momentum.GetCoordinateSystem()}
    , rotatedCS_{originalCS_.RotateToZ(momentum)} {
  auto const squaredNorm = momentum.squaredNorm();
  auto const norm = sqrt(squaredNorm);
  auto const sinhEta = -norm / mass;
  auto const coshEta = sqrt(1 + squaredNorm / (mass * mass));
  setBoost(coshEta, sinhEta);
}

void COMBoost::setBoost(double coshEta, double sinhEta) {
  boost_ << coshEta, sinhEta, sinhEta, coshEta;
  inverseBoost_ << coshEta, -sinhEta, -sinhEta, coshEta;
}

CoordinateSystem const& COMBoost::GetRotatedCS() const { return rotatedCS_; }
