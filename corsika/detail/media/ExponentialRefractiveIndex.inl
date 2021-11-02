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
      double const n0, InverseLengthType const lambda, Point const center, LengthType const radius, Args&&... args)
      : T(std::forward<Args>(args)...)
      , n0_(n0)
      , lambda_(lambda)
      , center_(center)
      , radius_(radius) {}

  template <typename T>
  double ExponentialRefractiveIndex<T>::getRefractiveIndex(Point const& point) const {
    return n0_ * exp((-lambda_) * (distance(point, center_) - radius_));
  }

} // namespace corsika
