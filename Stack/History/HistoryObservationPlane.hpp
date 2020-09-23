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

namespace corsika::history {
  namespace detail {
    auto hist_factory() {
      /*auto h = boost::histogram::make_histogram(
          boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>{
              130, 1e8, 1e21, "muon energy/eV"},
          boost::histogram::axis::integer<int, boost::histogram::use_default,
                                          boost::histogram::axis::option::growth_t>{
              0, 10, "hadronic generation"},
          boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>{
              130, 1e8, 1e21, "hadronic energy/eV"},
          boost::histogram::axis::category<int, boost::histogram::use_default,
                                           boost::histogram::axis::option::growth_t>{});*/

      auto h = boost::histogram::make_histogram(
          boost::histogram::axis::integer<int, boost::histogram::use_default,
                                          boost::histogram::axis::option::growth_t>{
              0, 10, "hadronic generation"});
      return h;
    }
  } // namespace detail

  class HistoryObservationPlane
      : public corsika::process::ContinuousProcess<HistoryObservationPlane> {
  public:
    HistoryObservationPlane(setup::Stack const&, geometry::Plane const&, bool = true);

    //~ void save(std::string const&);

    corsika::units::si::LengthType MaxStepLength(
        corsika::setup::Stack::ParticleType const&,
        corsika::setup::Trajectory const& vTrajectory);

    corsika::process::EProcessReturn DoContinuous(
        corsika::setup::Stack::ParticleType const& vParticle,
        corsika::setup::Trajectory const& vTrajectory);

  private:
    void fillHistoryHistogram(setup::Stack::ParticleType const&);

    setup::Stack const& stack_;
    geometry::Plane const plane_;
    bool const deleteOnHit_;

    decltype(detail::hist_factory()) histogram_ = detail::hist_factory();
  };
} // namespace corsika::history
