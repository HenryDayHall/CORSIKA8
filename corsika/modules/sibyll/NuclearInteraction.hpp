/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>

namespace corsika::sibyll {

  class Interaction; // fwd-decl

  /**
   *
   *
   **/
  template <class TEnvironment>
  class NuclearInteraction
      : public corsika::InteractionProcess<NuclearInteraction<TEnvironment>> {

    int count_ = 0;
    int nucCount_ = 0;

  public:
    NuclearInteraction(corsika::sibyll::Interaction&, TEnvironment const&);
    ~NuclearInteraction();

    void Init();

    void InitializeNuclearCrossSections();
    void PrintCrossSectionTable(corsika::Code);
    CrossSectionType ReadCrossSectionTable(const int, corsika::Code, HEPEnergyType);
    HEPEnergyType GetMinEnergyPerNucleonCoM() { return gMinEnergyPerNucleonCoM_; }
    HEPEnergyType GetMaxEnergyPerNucleonCoM() { return gMaxEnergyPerNucleonCoM_; }
    unsigned int constexpr GetMaxNucleusAProjectile() { return gMaxNucleusAProjectile_; }
    unsigned int constexpr GetMaxNFragments() { return gMaxNFragments_; }
    unsigned int constexpr GetNEnergyBins() { return gNEnBins_; }

    template <typename Particle>
    std::tuple<CrossSectionType, CrossSectionType> GetCrossSection(
        Particle const& p, const corsika::Code TargetId);

    template <typename Particle>
    GrammageType GetInteractionLength(Particle const&);

    template <typename TSecondaryView>
    corsika::EProcessReturn DoInteraction(TSecondaryView&);

  private:
    TEnvironment const& environment_;
    corsika::sibyll::Interaction& hadronicInteraction_;
    std::map<corsika::Code, int> targetComponentsIndex_;
    corsika::default_prng_type& RNG_ = corsika::RNGManager::getInstance().getRandomStream("s_rndm");
    static constexpr unsigned int gNSample_ =
        500; // number of samples in MC estimation of cross section
    static constexpr unsigned int gMaxNucleusAProjectile_ = 56;
    static constexpr unsigned int gNEnBins_ = 6;
    static constexpr unsigned int gMaxNFragments_ = 60;
    // energy limits defined by table used for cross section in signuc.f
    // 10**1 GeV to 10**6 GeV
    static constexpr HEPEnergyType gMinEnergyPerNucleonCoM_ = 10. * 1e9 * electronvolt;
    static constexpr HEPEnergyType gMaxEnergyPerNucleonCoM_ = 1.e6 * 1e9 * electronvolt;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/NuclearInteraction.inl>
