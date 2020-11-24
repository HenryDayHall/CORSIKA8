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

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

namespace corsika {

  template <typename TDimension>
  CoordinateSystemPtr BaseVector<TDimension>::getCoordinateSystem() const {
    return cs_;
  }

  template <typename TDimension>
  QuantityVector<TDimension> const& BaseVector<TDimension>::getQuantityVector() const {
    return quantityVector_;
  }

  template <typename TDimension>
  QuantityVector<TDimension>& BaseVector<TDimension>::quantityVector() {
    return quantityVector_;
  }

} // namespace corsika
