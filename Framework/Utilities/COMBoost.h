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

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/units/PhysicalUnits.h>

#include <Eigen/Dense>

namespace corsika::utl {

  /**
     This utility class handles Lorentz boost between different
     referenence frames, using FourVectors.
   */

  class COMBoost {
    Eigen::Matrix2d boost_, inverseBoost_;
    corsika::geometry::CoordinateSystem const &originalCS_, rotatedCS_;

    void setBoost(double coshEta, double sinhEta);

  public:
    //! construct a COMBoost given four-vector of projectile and mass of target
    COMBoost(
        const corsika::geometry::FourVector<
            corsika::units::si::HEPEnergyType,
            corsika::geometry::Vector<corsika::units::si::hepmomentum_d>>& Pprojectile,
        const corsika::units::si::HEPEnergyType massTarget);

    //! construct a COMBoost to boost into the rest frame given a 3-momentum and mass
    COMBoost(geometry::Vector<units::si::hepmomentum_d> const& momentum,
             units::si::HEPEnergyType mass);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    template <typename FourVector>
    FourVector toCoM(const FourVector& p) const {
      using namespace corsika::units::si;
      auto pComponents = p.GetSpaceLikeComponents().GetComponents(rotatedCS_);
      Eigen::Vector3d eVecRotated = pComponents.eVector;
      Eigen::Vector2d lab;

      lab << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
          (eVecRotated(2) * (1 / 1_GeV).magnitude());

      auto const boostedZ = boost_ * lab;
      auto const E_CoM = boostedZ(0) * 1_GeV;

      eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

      return FourVector(
          E_CoM, corsika::geometry::Vector<hepmomentum_d>(rotatedCS_, eVecRotated));
    }

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    template <typename FourVector>
    FourVector fromCoM(const FourVector& p) const {
      using namespace corsika::units::si;
      auto pCM = p.GetSpaceLikeComponents().GetComponents(rotatedCS_);
      auto const Ecm = p.GetTimeLikeComponent();

      Eigen::Vector2d com;
      com << (Ecm * (1 / 1_GeV)), (pCM.eVector(2) * (1 / 1_GeV).magnitude());

      std::cout << "COMBoost::fromCoM Ecm=" << Ecm / 1_GeV << " GeV, "
                << " pcm = " << pCM / 1_GeV << " (norm = " << pCM.norm() / 1_GeV
                << " GeV), invariant mass = " << p.GetNorm() / 1_GeV << " GeV"
                << std::endl;

      auto const boostedZ = inverseBoost_ * com;
      auto const E_lab = boostedZ(0) * 1_GeV;

      pCM.eVector(2) = boostedZ(1) * (1_GeV).magnitude();

      geometry::Vector<typename decltype(pCM)::dimension> pLab{rotatedCS_, pCM};
      pLab.rebase(originalCS_);

      FourVector f(E_lab, pLab);

      std::cout << "COMBoost::fromCoM --> Elab=" << E_lab / 1_GeV << "GeV, "
                << " plab = " << pLab.GetComponents() << " (norm =" << pLab.norm() / 1_GeV
                << " GeV), invariant mass = " << f.GetNorm() / 1_GeV << " GeV"
                << std::endl;

      return f;
    }

    geometry::CoordinateSystem const& GetRotatedCS() const;
  };
} // namespace corsika::utl
