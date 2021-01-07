/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/modules/LongitudinalProfile.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <cmath>
#include <iomanip>
#include <limits>

namespace corsika {

  LongitudinalProfile::LongitudinalProfile(ShowerAxis const& shower_axis, GrammageType dX)
      : dX_(dX)
      , shower_axis_{shower_axis}
      , profiles_{static_cast<unsigned int>(shower_axis.getMaximumX() / dX_) + 1} {}

  template <typename TParticle, typename TTrack>
  ProcessReturn LongitudinalProfile::doContinuous(TParticle const& vP,
                                                  TTrack const& vTrack) {
    auto const pid = vP.getPID();

    GrammageType const grammageStart = shower_axis_.getProjectedX(vTrack.getPosition(0));
    GrammageType const grammageEnd = shower_axis_.getProjectedX(vTrack.getPosition(1));

    CORSIKA_LOG_INFO(
        "pos1={} m, pos2={}, X={} g/cm2", vTrack.getPosition(0).getCoordinates() / 1_m,
        vTrack.getPosition(1).getCoordinates() / 1_m, grammageStart / 1_g * square(1_cm));

    const int binStart = std::ceil(grammageStart / dX_);
    const int binEnd = std::floor(grammageEnd / dX_);

    for (int b = binStart; b <= binEnd; ++b) {
      if (pid == Code::Gamma) {
        profiles_.at(b)[ProfileIndex::Gamma]++;
      } else if (pid == Code::Positron) {
        profiles_.at(b)[ProfileIndex::Positron]++;
      } else if (pid == Code::Electron) {
        profiles_.at(b)[ProfileIndex::Electron]++;
      } else if (pid == Code::MuPlus) {
        profiles_.at(b)[ProfileIndex::MuPlus]++;
      } else if (pid == Code::MuMinus) {
        profiles_.at(b)[ProfileIndex::MuMinus]++;
      } else if (is_hadron(pid)) {
        profiles_.at(b)[ProfileIndex::Hadron]++;
      }
    }

    return ProcessReturn::Ok;
  }

  void LongitudinalProfile::save(std::string const& filename, const int width,
                                 const int precision) {
    std::ofstream f{filename};
    f << "# X / g·cm¯², gamma, e+, e-, mu+, mu-, all hadrons" << std::endl;
    for (size_t b = 0; b < profiles_.size(); ++b) {
      f << std::setprecision(5) << std::setw(11) << b * (dX_ / (1_g / 1_cm / 1_cm));
      for (auto const& N : profiles_.at(b)) {
        f << std::setw(width) << std::setprecision(precision) << std::scientific << N;
      }
      f << std::endl;
    }
  }
} // namespace corsika
