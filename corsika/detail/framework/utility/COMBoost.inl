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
#include <corsika/framework/logging/Logging.hpp>

// using namespace corsika::units::si;

namespace corsika {

  COMBoost::COMBoost(FourVector<HEPEnergyType, Vector<hepmomentum_d>> const& Pprojectile,
                     HEPMassType const massTarget)
      : originalCS_{Pprojectile.getSpaceLikeComponents().getCoordinateSystem()}
      , rotatedCS_{originalCS_->rotateToZ(Pprojectile.getSpaceLikeComponents())} {
    auto const pProjectile = Pprojectile.getSpaceLikeComponents();
    auto const pProjNormSquared = pProjectile.getSquaredNorm();
    auto const pProjNorm = sqrt(pProjNormSquared);

    auto const eProjectile = Pprojectile.getTimeLikeComponent();
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

  COMBoost::COMBoost(Vector<hepmomentum_d> const& momentum, HEPEnergyType mass)
      : originalCS_{momentum.getCoordinateSystem()}
      , rotatedCS_{originalCS_->rotateToZ(momentum)} {
    auto const squaredNorm = momentum.getSquaredNorm();
    auto const norm = sqrt(squaredNorm);
    auto const sinhEta = -norm / mass;
    auto const coshEta = sqrt(1 + squaredNorm / (mass * mass));
    setBoost(coshEta, sinhEta);
  }

  template <typename FourVector>
  FourVector COMBoost::toCoM(FourVector const& p) const {
    auto pComponents = p.getSpaceLikeComponents().getComponents(rotatedCS_);
    Eigen::Vector3d eVecRotated = pComponents.getEigenVector();
    Eigen::Vector2d lab;

    lab << (p.getTimeLikeComponent() * (1 / 1_GeV)),
        (eVecRotated(2) * (1 / 1_GeV).magnitude());

    auto const boostedZ = boost_ * lab;
    auto const E_CoM = boostedZ(0) * 1_GeV;

    eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

    return FourVector(E_CoM, Vector<hepmomentum_d>(rotatedCS_, eVecRotated));
  }

  template <typename FourVector>
  FourVector COMBoost::fromCoM(FourVector const& p) const {
    auto pCM = p.getSpaceLikeComponents().getComponents(rotatedCS_);
    auto const Ecm = p.getTimeLikeComponent();

    Eigen::Vector2d com;
    com << (Ecm * (1 / 1_GeV)), (pCM.getEigenVector()(2) * (1 / 1_GeV).magnitude());

    CORSIKA_LOG_TRACE(
        "COMBoost::fromCoM Ecm={} GeV"
        " pcm={} GeV (norm = {} GeV), invariant mass={} GeV",
        Ecm / 1_GeV, pCM / 1_GeV, pCM.getNorm() / 1_GeV, p.getNorm() / 1_GeV);

    auto const boostedZ = inverseBoost_ * com;
    auto const E_lab = boostedZ(0) * 1_GeV;

    pCM.eigenVector()(2) = boostedZ(1) * (1_GeV).magnitude();

    Vector<typename decltype(pCM)::dimension_type> pLab{rotatedCS_, pCM};
    pLab.rebase(originalCS_);

    FourVector f(E_lab, pLab);

    CORSIKA_LOG_TRACE("COMBoost::fromCoM --> Elab={} GeV",
                      " plab={} GeV (norm={} GeV) "
                      " GeV), invariant mass = {}",
                      E_lab / 1_GeV, f.getNorm() / 1_GeV, pLab.getComponents(),
                      pLab.getNorm() / 1_GeV);

    return f;
  }

  void COMBoost::setBoost(double coshEta, double sinhEta) {
    boost_ << coshEta, sinhEta, sinhEta, coshEta;
    inverseBoost_ << coshEta, -sinhEta, -sinhEta, coshEta;
  }

  CoordinateSystemPtr COMBoost::getRotatedCS() const { return rotatedCS_; }

} // namespace corsika
