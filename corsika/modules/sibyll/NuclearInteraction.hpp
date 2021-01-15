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
  class NuclearInteraction : public InteractionProcess<NuclearInteraction<TEnvironment>> {

  public:
    NuclearInteraction(sibyll::Interaction&, TEnvironment const&);
    ~NuclearInteraction();

    void initializeNuclearCrossSections();
    void printCrossSectionTable(Code);
    CrossSectionType readCrossSectionTable(int const, Code const, HEPEnergyType const);
    HEPEnergyType getMinEnergyPerNucleonCoM() { return gMinEnergyPerNucleonCoM_; }
    HEPEnergyType getMaxEnergyPerNucleonCoM() { return gMaxEnergyPerNucleonCoM_; }
    unsigned int constexpr getMaxNucleusAProjectile() { return gMaxNucleusAProjectile_; }
    unsigned int constexpr getMaxNFragments() { return gMaxNFragments_; }
    unsigned int constexpr getNEnergyBins() { return gNEnBins_; }

    template <typename Particle>
    std::tuple<CrossSectionType, CrossSectionType> getCrossSection(Particle const& p,
                                                                   const Code TargetId);

    template <typename Particle>
    GrammageType getInteractionLength(Particle const&);

    template <typename TSecondaryView>
    void doInteraction(TSecondaryView&);

  private:
    int count_ = 0;
    int nucCount_ = 0;

    TEnvironment const& environment_;
    sibyll::Interaction& hadronicInteraction_;
    std::map<Code, int> targetComponentsIndex_;
    default_prng_type& RNG_ = RNGManager::getInstance().getRandomStream("sibyll");
    static unsigned int constexpr gNSample_ =
        500; // number of samples in MC estimation of cross section
    static unsigned int constexpr gMaxNucleusAProjectile_ = 56;
    static unsigned int constexpr gNEnBins_ = 6;
    static unsigned int constexpr gMaxNFragments_ = 60;
    // energy limits defined by table used for cross section in signuc.f
    // 10**1 GeV to 10**6 GeV
    static HEPEnergyType constexpr gMinEnergyPerNucleonCoM_ = 10. * 1e9 * electronvolt;
    static HEPEnergyType constexpr gMaxEnergyPerNucleonCoM_ = 1.e6 * 1e9 * electronvolt;
  };

} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/NuclearInteraction.inl>
