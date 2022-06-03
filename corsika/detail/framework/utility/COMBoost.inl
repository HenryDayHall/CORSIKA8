/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  inline COMBoost::COMBoost(FourMomentum const& P4projectile,
                            HEPMassType const massTarget)
      : originalCS_{P4projectile.getSpaceLikeComponents().getCoordinateSystem()}
      , rotatedCS_{make_rotationToZ(originalCS_, P4projectile.getSpaceLikeComponents())} {
    auto const& pProjectile = P4projectile.getSpaceLikeComponents();
    auto const pProjNormSquared = pProjectile.getSquaredNorm();
    auto const pProjNorm = sqrt(pProjNormSquared);

    auto const eProjectile = P4projectile.getTimeLikeComponent();
    auto const massProjectileSquared = eProjectile * eProjectile - pProjNormSquared;
    auto const s =
        massTarget * massTarget + massProjectileSquared + 2 * eProjectile * massTarget;

    auto const sqrtS = sqrt(s);
    auto const sinhEta = -pProjNorm / sqrtS;
    auto const coshEta = sqrt(1 + pProjNormSquared / s);

    setBoost(coshEta, sinhEta);
    CORSIKA_LOG_TRACE("COMBoost (1-beta)={}, gamma={}, det={}", 1 - sinhEta / coshEta,
                      coshEta, boost_.determinant() - 1);
  }

  inline COMBoost::COMBoost(FourMomentum const& P4projectile,
                            FourMomentum const& P4target)
      : originalCS_{P4projectile.getSpaceLikeComponents().getCoordinateSystem()} {

    // this is the center-of-momentum CM frame
    auto const pCM =
        P4projectile.getSpaceLikeComponents() + P4target.getSpaceLikeComponents();
    auto const pCM2 = pCM.getSquaredNorm();
    auto const pCMnorm = sqrt(pCM2);
    if (pCMnorm == 0_eV) {
      // CM is at reset
      rotatedCS_ = originalCS_;
    } else {
      rotatedCS_ = make_rotationToZ(originalCS_, P4projectile.getSpaceLikeComponents() +
                                                     P4target.getSpaceLikeComponents());
    }

    auto const s = (P4projectile + P4target).getNormSqr();
    auto const sqrtS = sqrt(s);
    auto const sinhEta = -pCMnorm / sqrtS;
    auto const coshEta = sqrt(1 + pCM2 / s);

    setBoost(coshEta, sinhEta);
    CORSIKA_LOG_TRACE("COMBoost (1-beta)={}, gamma={}, det={}", 1 - sinhEta / coshEta,
                      coshEta, boost_.determinant() - 1);
  }

  inline COMBoost::COMBoost(MomentumVector const& momentum, HEPEnergyType const mass)
      : originalCS_{momentum.getCoordinateSystem()}
      , rotatedCS_{make_rotationToZ(momentum.getCoordinateSystem(), momentum)} {
    auto const squaredNorm = momentum.getSquaredNorm();
    auto const norm = sqrt(squaredNorm);
    auto const sinhEta = -norm / mass;
    auto const coshEta = sqrt(1 + squaredNorm / (mass * mass));
    setBoost(coshEta, sinhEta);
    CORSIKA_LOG_TRACE("COMBoost (1-beta)={}, gamma={}, det={}", 1 - sinhEta / coshEta,
                      coshEta, boost_.determinant() - 1);
  }

  template <typename FourVector>
  inline FourVector COMBoost::toCoM(FourVector const& p4) const {
    auto const pComponents = p4.getSpaceLikeComponents().getComponents(rotatedCS_);
    Eigen::Vector3d eVecRotated = pComponents.getEigenVector();
    Eigen::Vector2d lab;

    lab << (p4.getTimeLikeComponent() * (1 / 1_GeV)),
        (eVecRotated(2) * (1 / 1_GeV).magnitude());

    auto const boostedZ = boost_ * lab;
    auto const E_CoM = boostedZ(0) * 1_GeV;

    eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

    CORSIKA_LOG_TRACE("E0={}, p={}, E0'={}, p'={}", p4.getTimeLikeComponent() / 1_GeV,
                      eVecRotated(2) * (1 / 1_GeV).magnitude(), E_CoM / 1_GeV, boostedZ);

    return FourVector(E_CoM, MomentumVector(rotatedCS_, eVecRotated));
  }

  template <typename FourVector>
  inline FourVector COMBoost::fromCoM(FourVector const& p4) const {
    auto pCM = p4.getSpaceLikeComponents().getComponents(rotatedCS_);
    auto const Ecm = p4.getTimeLikeComponent();

    Eigen::Vector2d com;
    com << (Ecm * (1 / 1_GeV)), (pCM.getEigenVector()(2) * (1 / 1_GeV).magnitude());

    CORSIKA_LOG_TRACE("Ecm={} GeV, pcm={} GeV (norm = {} GeV), invariant mass={} GeV",
                      Ecm / 1_GeV, pCM / 1_GeV, pCM.getNorm() / 1_GeV,
                      p4.getNorm() / 1_GeV);

    auto const boostedZ = inverseBoost_ * com;
    auto const E_lab = boostedZ(0) * 1_GeV;

    pCM.getEigenVector()[2] = boostedZ(1) * (1_GeV).magnitude();

    Vector<typename decltype(pCM)::dimension_type> pLab{rotatedCS_, pCM};
    pLab.rebase(originalCS_);

    CORSIKA_LOG_TRACE("COMBoost::fromCoM --> Elab={} GeV",
                      " plab={} GeV (norm={} GeV) "
                      " GeV), invariant mass = {}",
                      E_lab / 1_GeV, FourVector{E_lab, pLab}.getNorm() / 1_GeV,
                      pLab.getComponents(), pLab.getNorm() / 1_GeV);

    return FourVector{E_lab, pLab};
  }

  inline void COMBoost::setBoost(double const coshEta, double const sinhEta) {
    boost_ << coshEta, sinhEta, sinhEta, coshEta;
    inverseBoost_ << coshEta, -sinhEta, -sinhEta, coshEta;
  }

  inline CoordinateSystemPtr const& COMBoost::getRotatedCS() const { return rotatedCS_; }

  inline CoordinateSystemPtr const& COMBoost::getOriginalCS() const {
    return originalCS_;
  }

} // namespace corsika
