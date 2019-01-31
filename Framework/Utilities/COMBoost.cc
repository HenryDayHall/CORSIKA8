
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
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

using namespace corsika::utl;
using namespace corsika::units::si;

template <typename FourVector>
COMBoost<FourVector>::COMBoost(const FourVector& Pprojectile,
                               const HEPMassType massTarget)
    : fRotation(Eigen::Matrix3d::Identity())
    , fCS(Pprojectile.GetSpaceLikeComponents().GetCoordinateSystem()) {
  // calculate matrix for rotating pProjectile to z-axis first
  auto const pProjectile = Pprojectile.GetSpaceLikeComponents();
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
  double const beta = pProjNorm / (Pprojectile.GetTimeLikeComponent() + massTarget);

  /* Accurracy matters here, beta = 1 - epsilon for ultra-relativistic boosts */
  double const coshEta = 1 / std::sqrt((1 + beta) * (1 - beta));
  //~ double const coshEta = 1 / std::sqrt((1-beta*beta));
  double const sinhEta = -beta * coshEta;

  std::cout << "COMBoost (1-beta)=" << 1 - beta << " gamma=" << coshEta << std::endl;

  fBoost << coshEta, sinhEta, sinhEta, coshEta;

  fInverseBoost << coshEta, -sinhEta, -sinhEta, coshEta;
}

template <typename FourVector>
FourVector COMBoost<FourVector>::toCoM(const FourVector& p) const {
  auto pComponents = p.GetSpaceLikeComponents().GetComponents(fCS);
  Eigen::Vector3d eVecRotated = fRotation * pComponents.eVector;
  Eigen::Vector2d lab;

  lab << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
      (eVecRotated(2) * (1 / 1_GeV).magnitude());

  auto const boostedZ = fBoost * lab;
  auto const E_CoM = boostedZ(0) * 1_GeV;

  eVecRotated(2) = boostedZ(1) * (1_GeV).magnitude();

  return FourVector(E_CoM, corsika::geometry::Vector<hepmomentum_d>(fCS, eVecRotated));
}

template <typename FourVector>
FourVector COMBoost<FourVector>::fromCoM(const FourVector& p) const {
  Eigen::Vector2d com;
  com << (p.GetTimeLikeComponent() * (1 / 1_GeV)),
      (p.GetSpaceLikeComponents().GetComponents().eVector(2) * (1 / 1_GeV).magnitude());

  std::cout << "COMBoost::fromCoM Ecm=" << p.GetTimeLikeComponent() / 1_GeV << " GeV, "
            << " pcm=" << p.GetSpaceLikeComponents().GetComponents().squaredNorm() / 1_GeV
            << " GeV" << std::endl;

  auto const boostedZ = fInverseBoost * com;
  auto const E_lab = boostedZ(0) * 1_GeV;

  auto pLab = p.GetSpaceLikeComponents().GetComponents();
  pLab.eVector(2) = boostedZ(1) * (1_GeV).magnitude();
  pLab.eVector = fRotation.transpose() * pLab.eVector;

  std::cout << "COMBoost::fromCoM --> Elab=" << E_lab / 1_GeV << "GeV, "
            << " pcm=" << pLab.squaredNorm() / 1_GeV << "GeV" << std::endl;

  return FourVector(E_lab, corsika::geometry::Vector(fCS, pLab));
}

/*
  Here we instantiate all physically meaningful versions of COMBoost
 */

#include <corsika/geometry/FourVector.h>

namespace corsika::utl {

  using corsika::geometry::FourVector;
  using corsika::geometry::Vector;
  using namespace corsika::units;

  // for normal HEP energy/momentum units in [GeV]
  template class COMBoost<FourVector<HEPEnergyType, Vector<hepmomentum_d>>>;
  // template class COMBoost<FourVector<HEPEnergyType&, Vector<hepmomentum_d>&>>;

  // template class COMBoost<FourVector<TimeType, Vector<length_d>>>;
  // template class COMBoost<FourVector<TimeType&, Vector<length_d>&>>;
} // namespace corsika::utl
