/*
 * (c) Copyright 2025 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

namespace corsika {
  namespace media {

    SphericalTransformer::SphericalTransformer(LengthType radius) { radius_ = radius; }

    LengthType SphericalTransformer::getEffectiveHeight(Point const&) const {
      distance(point, center_).magnitude() -
          radius_; // If from surface a simple position.z() would work and be much
                   // faster;
    }

  } // namespace media
} // namespace corsika