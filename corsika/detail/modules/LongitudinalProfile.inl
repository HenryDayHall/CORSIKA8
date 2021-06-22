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

#include <cmath>
#include <iomanip>
#include <limits>

namespace corsika {

  inline LongitudinalProfile::LongitudinalProfile(ShowerAxis const& shower_axis,
                                                  GrammageType dX)
      : dX_(dX)
      , shower_axis_{shower_axis}
      , profiles_{static_cast<unsigned int>(shower_axis.getMaximumX() / dX_) + 1} {}

  template <typename TParticle, typename TTrack>
  inline ProcessReturn LongitudinalProfile::doContinuous(TParticle const& vP,
                                                         TTrack const& vTrack,
                                                         bool const) {
    auto const pid = vP.getPID();

    GrammageType const grammageStart = shower_axis_.getProjectedX(vTrack.getPosition(0));
    GrammageType const grammageEnd = shower_axis_.getProjectedX(vTrack.getPosition(1));

    CORSIKA_LOG_DEBUG("longprof: pos1={} m, pos2={}, X1={} g/cm2, X2={} g/cm2",
                      vTrack.getPosition(0).getCoordinates() / 1_m,
                      vTrack.getPosition(1).getCoordinates() / 1_m,
                      grammageStart / 1_g * square(1_cm),
                      grammageEnd / 1_g * square(1_cm));

    // Note: particle may go also "upward", thus, grammageEnd<grammageStart
    int const binStart = std::ceil(grammageStart / dX_);
    int const binEnd = std::floor(grammageEnd / dX_);

    for (int b = binStart; b <= binEnd; ++b) {
      if (pid == Code::Photon) {
        profiles_.at(b)[ProfileIndex::Photon]++;
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
      } else if (is_neutrino(pid)) {
        profiles_.at(b)[ProfileIndex::Invisible]++;
      }
    }

    return ProcessReturn::Ok;
  }

  inline void LongitudinalProfile::save(std::string const& filename, const int width,
                                        const int precision) {
    CORSIKA_LOG_DEBUG("Write longprof to {}", filename);
    std::ofstream f{filename};
    f << "# X / g·cm¯², photon, e+, e-, mu+, mu-, all hadrons, neutrinos" << std::endl;
    for (size_t b = 0; b < profiles_.size(); ++b) {
      f << std::setprecision(5) << std::setw(11) << b * (dX_ / (1_g / 1_cm / 1_cm));
      for (auto const& N : profiles_.at(b)) {
        f << std::setw(width) << std::setprecision(precision) << std::scientific << N;
      }
      f << std::endl;
    }
  }
} // namespace corsika
