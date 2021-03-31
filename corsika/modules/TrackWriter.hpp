/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/modules/writers/TrackWriterParquet.hpp>

namespace corsika {

  template <typename TOutputWriter = TrackWriterParquet>
  class TrackWriter : public ContinuousProcess<TrackWriter<TOutputWriter>>,
                      public TOutputWriter {

  public:
    TrackWriter();

    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(TParticle const&, TTrack const&);

    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const&, TTrack const&);

    YAML::Node getConfig() const;
  };

} // namespace corsika

#include <corsika/detail/modules/TrackWriter.inl>
