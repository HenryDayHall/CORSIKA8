/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Plane.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <boost/histogram.hpp>

#include <functional>

// the detail namespace: here the histrograms are defined
//! \todo add options/parameters to 'detail::hist_factory()'
#include <corsika/detail/stack/history/HistoryObservationPlane.hpp>

namespace corsika::history {

  class HistoryObservationPlane : public ContinuousProcess<HistoryObservationPlane> {
  public:
    HistoryObservationPlane(setup::Stack const&, Plane const&, bool = true);

    LengthType getMaxStepLength(setup::Stack::particle_type const&,
                                setup::Trajectory const& vTrajectory);

    ProcessReturn doContinuous(setup::Stack::particle_type const& vParticle,
                               setup::Trajectory const& vTrajectory);

    auto const& histogram() const { return histogram_; }

  private:
    void fillHistoryHistogram(setup::Stack::particle_type const&);

    setup::Stack const& stack_;
    Plane const plane_;
    bool const deleteOnHit_;

    decltype(detail::hist_factory()) histogram_ = detail::hist_factory();
  };
} // namespace corsika::history

#include <corsika/detail/stack/history/HistoryObservationPlane.inl>
