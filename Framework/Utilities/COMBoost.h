/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_utilties_comboost_h_
#define _include_corsika_utilties_comboost_h_

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
    Eigen::Matrix3d fRotation;
    Eigen::Matrix2d fBoost, fInverseBoost;
    corsika::geometry::CoordinateSystem const& fCS;

  public:
    //! construct a COMBoost given four-vector of prjectile and mass of target
    COMBoost(
        const corsika::geometry::FourVector<
            corsika::units::si::HEPEnergyType,
            corsika::geometry::Vector<corsika::units::si::hepmomentum_d>>& Pprojectile,
        const corsika::units::si::HEPEnergyType massTarget);

    auto const& GetRotationMatrix() const { return fRotation; }

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    template <typename FourVector>
    FourVector toCoM(const FourVector& p) const {
      using namespace corsika::units::si;
      auto pComponents = p.GetSpaceLikeComponents().GetComponents(fCS);
      Eigen::Vector3d eVecRotated = fRotation * pComponents.eVector;
      Eigen::Vector2d lab;

      lab << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
          (eVecRotated(2) * (1 / 1_GeV).magnitude());

      auto const boostedZ = fBoost * lab;
      auto const E_CoM = boostedZ(0) * 1_GeV;

      eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

      return FourVector(E_CoM,
                        corsika::geometry::Vector<hepmomentum_d>(fCS, eVecRotated));
    }

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    template <typename FourVector>
    FourVector fromCoM(const FourVector& p) const {
      using namespace corsika::units::si;
      Eigen::Vector2d com;
      com << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
          (p.GetSpaceLikeComponents().GetComponents().eVector(2) *
           (1 / 1_GeV).magnitude());

      auto const plab = p.GetSpaceLikeComponents().GetComponents();
      std::cout << "COMBoost::fromCoM Ecm=" << p.GetTimeLikeComponent() / 1_GeV
                << " GeV, "
                << " pcm = " << plab / 1_GeV << " (norm = " << plab.norm() / 1_GeV
                << " GeV), invariant mass = " << p.GetNorm() / 1_GeV << " GeV"
                << std::endl;

      auto const boostedZ = fInverseBoost * com;
      auto const E_lab = boostedZ(0) * 1_GeV;

      auto pLab = p.GetSpaceLikeComponents().GetComponents();
      pLab.eVector(2) = boostedZ(1) * (1_GeV).magnitude();
      pLab.eVector = fRotation.transpose() * pLab.eVector;

      FourVector f(E_lab, corsika::geometry::Vector(fCS, pLab));

      std::cout << "COMBoost::fromCoM --> Elab=" << E_lab / 1_GeV << "GeV, "
                << " pcm = " << pLab / 1_GeV << " (norm =" << pLab.norm() / 1_GeV
                << " GeV), invariant mass = " << f.GetNorm() / 1_GeV << " GeV"
                << std::endl;

      return f;
    }
  };
} // namespace corsika::utl

#endif
