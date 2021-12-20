/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>

#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>

namespace corsika {

  template <typename TOutput>
  template <typename... TArgs>
  inline LongitudinalProfile<TOutput>::LongitudinalProfile(TArgs&&... args)
      : TOutput(std::forward<TArgs>(args)...) {}

  template <typename TOutput>
  template <typename TParticle, typename TTrack>
  inline ProcessReturn LongitudinalProfile<TOutput>::doContinuous(
      TParticle const& particle, TTrack const& track, bool const) {

    auto const pid = particle.getPID();
    this->write(track, pid, 1.0); // weight hardcoded so far
    return ProcessReturn::Ok;
  }

  template <typename TOutput>
  inline YAML::Node LongitudinalProfile<TOutput>::getConfig() const {
    YAML::Node node;
    node["type"] = "LongitudinalProfile";

    return node;
  }

} // namespace corsika
