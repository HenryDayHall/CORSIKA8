/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/ShowerAxis.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <array>
#include <fstream>
#include <limits>
#include <string>

namespace corsika {

  /**
   * \class LongitudinalProfile
   *
   * \todo test missing
   *
   * is a ContinuousProcess, which is constructed from an environment::ShowerAxis
   * object, and a dX in units of g/cm2  (GrammageType).
   *
   * LongitudinalProfile does then convert each single Track of the
   * simulation into a projected grammage range and counts for
   * different particle species when they cross dX (default: 10g/cm2)
   * boundaries.
   */

  class LongitudinalProfile : public ContinuousProcess<LongitudinalProfile> {

  public:
    LongitudinalProfile(ShowerAxis const&,
                        GrammageType dX = 10_g / square(1_cm)); // profile binning);

    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(
        TParticle const&, TTrack const&,
        bool const flagLimit = false); // not needed for LongitudinalProfile

    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const&, TTrack const&) {
      return meter * std::numeric_limits<double>::infinity();
    }

    void save(std::string const&, int const width = 14, int const precision = 6);

  private:
    GrammageType const dX_;
    ShowerAxis const& shower_axis_;
    using ProfileEntry = std::array<uint32_t, 7>;
    enum ProfileIndex {
      Photon = 0,
      Positron = 1,
      Electron = 2,
      MuPlus = 3,
      MuMinus = 4,
      Hadron = 5,
      Invisible = 6,
    };
    std::vector<ProfileEntry> profiles_; // longitudinal profile
  };

} // namespace corsika

#include <corsika/detail/modules/LongitudinalProfile.inl>
