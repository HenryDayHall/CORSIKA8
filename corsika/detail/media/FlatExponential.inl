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

#include <corsika/media/FlatExponential.hpp>

namespace corsika {

  template <class T>
  units::si::MassDensityType FlatExponential<T>::GetMassDensity(Point const& vP) const {
    return Base::fRho0 * exp(Base::fInvLambda * (vP - Base::fP0).dot(fAxis));
  }

  template <class T>
  NuclearComposition const& FlatExponential<T>::GetNuclearComposition() const {
    return fNuclComp;
  }

  template <class T>
  units::si::GrammageType FlatExponential<T>::IntegratedGrammage(
      Trajectory<Line> const& vLine, units::si::LengthType vTo) const {
    return Base::IntegratedGrammage(vLine, vTo, fAxis);
  }

  template <class T>
  units::si::LengthType FlatExponential<T>::ArclengthFromGrammage(
      Trajectory<Line> const& vLine, units::si::GrammageType vGrammage) const {
    return Base::ArclengthFromGrammage(vLine, vGrammage, fAxis);
  }

} // namespace corsika

#include <corsika/detail/media/BaseExponential.inl>
