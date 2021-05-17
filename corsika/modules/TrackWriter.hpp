/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/modules/writers/TrackWriterOff.hpp>
#include <corsika/modules/writers/TrackWriterParquet.hpp>

namespace corsika {

  template <typename TOutput = TrackWriterOff>
  class TrackWriter : public ContinuousProcess<TrackWriter<TOutput>> {

  public:
    TrackWriter(TOutput& output);

    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(TParticle const&, TTrack const&, bool const limitFlag);

    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const&, TTrack const&);

    YAML::Node getConfig() const;

  private:
    TOutput& output_;
  };

} // namespace corsika

#include <corsika/detail/modules/TrackWriter.inl>
