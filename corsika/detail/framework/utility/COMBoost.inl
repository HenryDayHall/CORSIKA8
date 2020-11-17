/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cmath>

#include <Eigen/Dense>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/logging/Logging.h>
#include <corsika/framework/utility/sgn.hpp>

// using namespace corsika::units::si;

namespace corsika {

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

    C8LOG_TRACE("COMBoost (1-beta)={}, gamma={}, det={}", 1 - sinhEta / coshEta, coshEta,
                boost_.determinant() - 1);
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

  template <typename FourVector>
  FourVector COMBoost::toCoM(const FourVector& p) const {
    using namespace corsika::units::si;
    auto pComponents = p.GetSpaceLikeComponents().GetComponents(rotatedCS_);
    Eigen::Vector3d eVecRotated = pComponents.eVector;
    Eigen::Vector2d lab;

    lab << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
        (eVecRotated(2) * (1 / 1_GeV).magnitude());

    auto const boostedZ = boost_ * lab;
    auto const E_CoM = boostedZ(0) * 1_GeV;

    eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

    return FourVector(E_CoM,
                      corsika::geometry::Vector<hepmomentum_d>(rotatedCS_, eVecRotated));
  }

  template <typename FourVector>
  FourVector COMBoost::fromCoM(const FourVector& p) const {
    using namespace corsika::units::si;
    auto pCM = p.GetSpaceLikeComponents().GetComponents(rotatedCS_);
    auto const Ecm = p.GetTimeLikeComponent();

    Eigen::Vector2d com;
    com << (Ecm * (1 / 1_GeV)), (pCM.eVector(2) * (1 / 1_GeV).magnitude());

    C8LOG_TRACE(
        "COMBoost::fromCoM Ecm={} GeV"
        " pcm={} GeV (norm = {} GeV), invariant mass={} GeV",
        Ecm / 1_GeV, pCM / 1_GeV, pCM.norm() / 1_GeV, p.GetNorm() / 1_GeV);

    auto const boostedZ = inverseBoost_ * com;
    auto const E_lab = boostedZ(0) * 1_GeV;

    pCM.eVector(2) = boostedZ(1) * (1_GeV).magnitude();

    geometry::Vector<typename decltype(pCM)::dimension> pLab{rotatedCS_, pCM};
    pLab.rebase(originalCS_);

    FourVector f(E_lab, pLab);

    C8LOG_TRACE("COMBoost::fromCoM --> Elab={} GeV",
                " plab={} GeV (norm={} GeV) "
                " GeV), invariant mass = {}",
                E_lab / 1_GeV, f.GetNorm() / 1_GeV, pLab.GetComponents(),
                pLab.norm() / 1_GeV);

    return f;
  }

  void COMBoost::setBoost(double coshEta, double sinhEta) {
    boost_ << coshEta, sinhEta, sinhEta, coshEta;
    inverseBoost_ << coshEta, -sinhEta, -sinhEta, coshEta;
  }

  CoordinateSystem const& COMBoost::GetRotatedCS() const { return rotatedCS_; }

} // namespace corsika
