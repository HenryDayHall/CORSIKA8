/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/longitudinal_profile/LongitudinalProfile.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <cmath>
#include <iomanip>
#include <limits>

using namespace corsika::setup;
using Particle = Stack::ParticleType;
using Track = Trajectory;

using namespace corsika::process::longitudinal_profile;
using namespace corsika::units::si;

LongitudinalProfile::LongitudinalProfile(environment::ShowerAxis const& shower_axis)
    : shower_axis_{shower_axis}
    , profiles_{int(shower_axis.maximumX() / dX_) + 1} {}

void LongitudinalProfile::Init() {}

template <>
corsika::process::EProcessReturn LongitudinalProfile::DoContinuous(Particle const& vP,
                                                                   Track const& vTrack) {
  auto const pid = vP.GetPID();

  GrammageType const grammageStart = shower_axis_.projectedX(vTrack.GetPosition(0));
  GrammageType const grammageEnd = shower_axis_.projectedX(vTrack.GetPosition(1));

  const int binStart = std::ceil(grammageStart / dX_);
  const int binEnd = std::floor(grammageEnd / dX_);

  for (int b = binStart; b <= binEnd; ++b) {
    if (pid == particles::Code::MuPlus) {
      profiles_.at(b)[ProfileIndex::MuPlus]++;
    } else if (pid == particles::Code::MuMinus) {
      profiles_.at(b)[ProfileIndex::MuMinus]++;
    } else if (particles::IsHadron(pid)) {
      profiles_.at(b)[ProfileIndex::Hadron]++;
    }
  }

  return corsika::process::EProcessReturn::eOk;
}

void LongitudinalProfile::save(std::string const& filename) {
  std::ofstream f{filename};
  f << "# X / g·cm¯², mu+, mu-, all hadrons" << std::endl;
  for (size_t b = 0; b < profiles_.size(); ++b) {
    f << std::setprecision(5) << std::setw(11) << b * (dX_ / (1_g / 1_cm / 1_cm));
    for (auto const& N : profiles_.at(b)) {
      f << std::setw(width_) << std::setprecision(precision_) << std::scientific << N;
    }
    f << std::endl;
  }
}
