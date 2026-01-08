/*
 * (c) Copyright 2025 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

namespace corsika {
  namespace media {

    LengthType PlanarTransformer::getEffectiveHeight(const PointType& position) const {
      return position.z();
    }

  } // namespace media
} // namespace corsika