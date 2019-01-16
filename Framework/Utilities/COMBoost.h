/**
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

#include <Eigen/Dense>

namespace corsika::utl {

  /**
     This utility class handles Lorentz boost between different
     referenence frames, using FourVectors.
   */

  template <typename FourVector>
  class COMBoost {
    Eigen::Matrix3d fRotation;
    Eigen::Matrix2d fBoost, fInverseBoost;
    corsika::geometry::CoordinateSystem const& fCS;

  public:
    //! construct a COMBoost given energy and momentum of projectile and mass of target
    COMBoost(const FourVector& Pprojectile, const FourVector& Ptarget);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    FourVector toCoM(const FourVector& p) const;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    FourVector fromCoM(const FourVector& pCoM) const;
  };
} // namespace corsika::utl

#endif
