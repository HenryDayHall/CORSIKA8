/*
 * (c) Copyright 2025 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

namespace corsika {
  namespace media {

    CylindricalTransformer::CylindricalTransformer(LengthType radius) {
      radius_ = radius;
    }

    LengthType CylindricalTransformer::getEffectiveHeight(
        const PointType& position) const {

      double ref = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
      Eigen::Vector3d normal = {ref, 0, pos[2] + radius_};

      return normal.norm() - radius_;
    }

  } // namespace media
} // namespace corsika