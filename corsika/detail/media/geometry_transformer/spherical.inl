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

    LengthType SphericalTransformer::getEffectiveHeight(const PointType& position) const {
      return (pos + Eigen::Vector3d(0., 0., radius_)).norm() - radius_;
    }

  } // namespace media
} // namespace corsika