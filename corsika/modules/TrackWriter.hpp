/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <fstream>
#include <string>

namespace corsika {

  class TrackWriter : public ContinuousProcess<TrackWriter> {

  public:
    TrackWriter(std::string const& filename);

    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(TParticle const&, TTrack const&);

    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const&, TTrack const&);

  private:
    std::string const filename_;
    std::ofstream file_;

    int width_ = 14;
    int precision_ = 6;
  };

} // namespace corsika

#include <corsika/detail/modules/TrackWriter.inl>
