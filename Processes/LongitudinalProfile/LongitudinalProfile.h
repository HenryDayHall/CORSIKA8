/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/ShowerAxis.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/units/PhysicalUnits.h>

#include <array>
#include <fstream>
#include <limits>
#include <string>

namespace corsika::process::longitudinal_profile {

  class LongitudinalProfile
      : public corsika::process::ContinuousProcess<LongitudinalProfile> {

  public:
    LongitudinalProfile(environment::ShowerAxis const&);

    void Init();

    template <typename Particle, typename Track>
    corsika::process::EProcessReturn DoContinuous(Particle const&, Track const&);

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle const&, Track const&) {
      return units::si::meter * std::numeric_limits<double>::infinity();
    }

    void save(std::string const&);

  private:
    units::si::GrammageType const dX_ = std::invoke([]() {
      using namespace units::si;
      return 10_g / square(1_cm);
    }); // profile binning

    environment::ShowerAxis const& shower_axis_;
    using ProfileEntry = std::array<uint32_t, 3>;
    enum ProfileIndex { MuPlus = 0, MuMinus = 1, Hadron = 2 };
    std::vector<ProfileEntry> profiles_; // longitudinal profile

    static int const width_ = 14;
    static int const precision_ = 6;
  };

} // namespace corsika::process::longitudinal_profile
