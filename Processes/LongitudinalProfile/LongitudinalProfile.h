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

  /**
   * \class LongitudinalProfile
   *
   * is a ContinuousProcess, which is constructed from an environment::ShowerAxis 
   * object, and a dX in units of g/cm2
   * (corsika::units::si::GrammageType). 
   *
   * LongitudinalProfile does then convert each single Track of the
   * simulation into a projected grammage range and counts for
   * different particle species when they cross dX (default: 10g/cm2)
   * boundaries.
   *
   **/

  class LongitudinalProfile
      : public corsika::process::ContinuousProcess<LongitudinalProfile> {

  public:
    LongitudinalProfile(environment::ShowerAxis const&,
                        units::si::GrammageType dX = std::invoke([]() {
                          using namespace units::si;
                          return 10_g / square(1_cm);
                        })); // profile binning);

    template <typename TParticle, typename TTrack>
    corsika::process::EProcessReturn DoContinuous(TParticle const&, TTrack const&);

    template <typename TParticle, typename TTrack>
    corsika::units::si::LengthType MaxStepLength(TParticle const&, TTrack const&) {
      return units::si::meter * std::numeric_limits<double>::infinity();
    }

    void save(std::string const&, int const width = 14, int const precision = 6);

  private:
    units::si::GrammageType const dX_;

    environment::ShowerAxis const& shower_axis_;
    using ProfileEntry = std::array<uint32_t, 6>;
    enum ProfileIndex {
      Gamma = 0,
      Positron = 1,
      Electron = 2,
      MuPlus = 3,
      MuMinus = 4,
      Hadron = 5
    };
    std::vector<ProfileEntry> profiles_; // longitudinal profile
  };

} // namespace corsika::process::longitudinal_profile
