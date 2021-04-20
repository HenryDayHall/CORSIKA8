/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/TimeCut.hpp>

namespace corsika {

  inline TimeCut::TimeCut(const TimeType time)
  : time_(time) {}

  inline ProcessReturn TimeCut::doContinuous(
      corsika::setup::Stack::particle_type& particle, corsika::setup::Trajectory const&,
      bool const) {
    CORSIKA_LOG_TRACE("TimeCut::doContinuous");
    if (particle.getTime() >= time_) {
      CORSIKA_LOG_TRACE("stopping continuous process");
      return ProcessReturn::ParticleAbsorbed;
    }
    return ProcessReturn::Ok;
  }

} // namespace corsika
