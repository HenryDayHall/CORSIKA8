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
    class CylindricalTransformer : public IGeometryTransformer {
    private:
      LengthType radius_;

    public:
      CylindricalTransformer(LengthType radius);
      ~CylindricalTransformer() = default;

      LengthType getEffectiveHeight(Point const& position) const {
        return position.z();
      }
    };

  } // namespace media
} // namespace corsika