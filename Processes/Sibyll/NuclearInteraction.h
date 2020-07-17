/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>

namespace corsika::process::sibyll {

  class Interaction; // fwd-decl

  /**
   *
   *
   **/
  template <class TEnvironment>
  class NuclearInteraction
      : public corsika::process::InteractionProcess<NuclearInteraction<TEnvironment>> {

    int count_ = 0;
    int nucCount_ = 0;

  public:
    NuclearInteraction(corsika::process::sibyll::Interaction&, TEnvironment const&);
    ~NuclearInteraction();
    void Init();
    void InitializeNuclearCrossSections();
    void PrintCrossSectionTable(corsika::particles::Code);
    corsika::units::si::CrossSectionType ReadCrossSectionTable(
        const int, corsika::particles::Code, corsika::units::si::HEPEnergyType);
    corsika::units::si::HEPEnergyType GetMinEnergyPerNucleonCoM() {
      return gMinEnergyPerNucleonCoM_;
    }
    corsika::units::si::HEPEnergyType GetMaxEnergyPerNucleonCoM() {
      return gMaxEnergyPerNucleonCoM_;
    }
    int constexpr GetMaxNucleusAProjectile() { return gMaxNucleusAProjectile_; }
    int constexpr GetMaxNFragments() { return gMaxNFragments_; }
    int constexpr GetNEnergyBins() { return gNEnBins_; }

    template <typename Particle>
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(Particle const& p, const corsika::particles::Code TargetId);

    template <typename Particle>
    corsika::units::si::GrammageType GetInteractionLength(Particle const&);

    template <typename Projectile>
    corsika::process::EProcessReturn DoInteraction(Projectile&);

  private:
    TEnvironment const& environment_;
    corsika::process::sibyll::Interaction& hadronicInteraction_;
    std::map<corsika::particles::Code, int> targetComponentsIndex_;
    corsika::random::RNG& RNG_ =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
    static constexpr int gNSample_ =
        500; // number of samples in MC estimation of cross section
    static constexpr int gMaxNucleusAProjectile_ = 56;
    static constexpr int gNEnBins_ = 6;
    static constexpr int gMaxNFragments_ = 60;
    // energy limits defined by table used for cross section in signuc.f
    // 10**1 GeV to 10**6 GeV
    static constexpr corsika::units::si::HEPEnergyType gMinEnergyPerNucleonCoM_ =
        10. * 1e9 * corsika::units::si::electronvolt;
    static constexpr corsika::units::si::HEPEnergyType gMaxEnergyPerNucleonCoM_ =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
  };

} // namespace corsika::process::sibyll
