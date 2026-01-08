/*
 * (c) Copyright 2025 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/media/interfaces/IGeometryTransformer.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/units/quantity.hpp>

namespace corsika {
  namespace media {
    class PlanarTransformer : public IGeometryTransformer {
    public:
      PlanarTransformer() = default;
      ~PlanarTransformer() = default;

      LengthType getEffectiveHeight(const PointType& position) const {
        return position.z();
      }
    };

  } // namespace media
} // namespace corsika

#include <corsika/detail/media/geometry_transformer/planar.inl>