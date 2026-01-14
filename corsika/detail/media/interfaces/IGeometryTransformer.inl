/*
 * (c) Copyright 2025 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

namespace corsika {
  namespace media {

    inline IGeometryTransformer::IGeometryTransformer(CoordinateSystemPtr const& cs)
        : cs_(cs) {}

  } // namespace media
} // namespace corsika
