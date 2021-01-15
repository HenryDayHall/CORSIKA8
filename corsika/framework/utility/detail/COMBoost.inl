/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/sgn.hpp>
#include <Eigen/Dense>
#include "../../core/PhysicalUnits.hpp"

namespace corsika::utl {

  auto const& COMBoost::GetRotationMatrix() const { return fRotation; }

  //! transforms a 4-momentum from lab frame to the center-of-mass frame
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

  //! transforms a 4-momentum from the center-of-mass frame back to lab frame
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
} // namespace corsika::utl
