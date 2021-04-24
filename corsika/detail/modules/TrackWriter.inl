/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <limits>

namespace corsika {

  template <typename TOutputWriter>
  inline TrackWriter<TOutputWriter>::TrackWriter() {}

  template <typename TOutputWriter>
  template <typename TParticle, typename TTrack>
  inline ProcessReturn TrackWriter<TOutputWriter>::doContinuous(TParticle const& vP,
                                                                TTrack const& vT,
                                                                bool const) {

    auto const start = vT.getPosition(0).getCoordinates();
    auto const end = vT.getPosition(1).getCoordinates();

    // write the track to the file
    this->write(vP.getPID(), vP.getEnergy(), start, end);

    return ProcessReturn::Ok;
  }

  template <typename TOutputWriter>
  template <typename TParticle, typename TTrack>
  inline LengthType TrackWriter<TOutputWriter>::getMaxStepLength(TParticle const&,
                                                                 TTrack const&) {
    return meter * std::numeric_limits<double>::infinity();
  }

  template <typename TOutputWriter>
  YAML::Node TrackWriter<TOutputWriter>::getConfig() const {
    using namespace units::si;

    YAML::Node node;

    // add default units for values
    node["type"] = "TrackWriter";
    node["units"] = "GeV | m";

    return node;
  }
} // namespace corsika
