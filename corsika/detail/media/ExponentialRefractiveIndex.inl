/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/IRefractiveIndexModel.hpp>

namespace corsika {

  template <typename T>
  template <typename... Args>
  ExponentialRefractiveIndex<T>::ExponentialRefractiveIndex(
      double const n0, InverseLengthType const lambda_, Point const center_, LengthType const planetRadius_, Args&&... args)
      : T(std::forward<Args>(args)...)
      , n_0(n0)
      , lambda(lambda_)
      , center(center_)
      , planetRadius(planetRadius_) {}

  template <typename T>
  double ExponentialRefractiveIndex<T>::getRefractiveIndex(Point const& point) const {
    return n_0 * exp((-lambda) * (distance(point, center) - planetRadius));
  }

} // namespace corsika
