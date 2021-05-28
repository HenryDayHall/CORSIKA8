/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/SlidingPlanarExponential.hpp>

namespace corsika {

  template <typename T>
  inline SlidingPlanarExponential<T>::SlidingPlanarExponential(
      Point const& p0, MassDensityType rho0, LengthType lambda,
      NuclearComposition const& nuclComp, LengthType referenceHeight)
      : BaseExponential<SlidingPlanarExponential<T>>(p0, rho0, lambda)
      , nuclComp_(nuclComp)
      , referenceHeight_(referenceHeight) {}

  template <typename T>
  inline MassDensityType SlidingPlanarExponential<T>::getMassDensity(
      Point const& point) const {
    auto const height =
        (point - BaseExponential<SlidingPlanarExponential<T>>::getAnchorPoint())
            .getNorm() -
        referenceHeight_;
    return BaseExponential<SlidingPlanarExponential<T>>::getRho0() *
           exp(BaseExponential<SlidingPlanarExponential<T>>::getInvLambda() * height);
  }

  template <typename T>
  inline NuclearComposition const& SlidingPlanarExponential<T>::getNuclearComposition()
      const {
    return nuclComp_;
  }

  template <typename T>
  inline GrammageType SlidingPlanarExponential<T>::getIntegratedGrammage(
      BaseTrajectory const& traj, LengthType l) const {
    auto const axis = (traj.getPosition(0) -
                       BaseExponential<SlidingPlanarExponential<T>>::getAnchorPoint())
                          .normalized();
    return BaseExponential<SlidingPlanarExponential<T>>::getIntegratedGrammage(traj, l,
                                                                               axis);
  }

  template <typename T>
  inline LengthType SlidingPlanarExponential<T>::getArclengthFromGrammage(
      BaseTrajectory const& traj, GrammageType const grammage) const {
    auto const axis = (traj.getPosition(0) -
                       BaseExponential<SlidingPlanarExponential<T>>::getAnchorPoint())
                          .normalized();
    return BaseExponential<SlidingPlanarExponential<T>>::getArclengthFromGrammage(
        traj, grammage, axis);
  }

} // namespace corsika
