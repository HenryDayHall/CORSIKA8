/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/geometry/QuantityVector.h>

namespace corsika {

  template <typename TDim>
  YAML::Emitter& operator<<(YAML::Emitter& out,
                            geometry::QuantityVector<TDim> const& vec) {
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
  }

} // namespace corsika
