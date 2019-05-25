/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_sibyll_nuclearinteraction_h_
#define _corsika_process_sibyll_nuclearinteraction_h_

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

    int fCount = 0;
    int fNucCount = 0;

  public:
    NuclearInteraction(corsika::process::sibyll::Interaction&, TEnvironment const&);
    ~NuclearInteraction();
    void Init();
    void InitializeNuclearCrossSections();
    void PrintCrossSectionTable(corsika::particles::Code);
    corsika::units::si::CrossSectionType ReadCrossSectionTable(
        const int, corsika::particles::Code, corsika::units::si::HEPEnergyType);
    corsika::units::si::HEPEnergyType GetMinEnergyPerNucleonCoM() {
      return gMinEnergyPerNucleonCoM;
    }
    corsika::units::si::HEPEnergyType GetMaxEnergyPerNucleonCoM() {
      return gMaxEnergyPerNucleonCoM;
    }
    int constexpr GetMaxNucleusAProjectile() { return gMaxNucleusAProjectile; }
    int constexpr GetMaxNFragments() { return gMaxNFragments; }
    int constexpr GetNEnergyBins() { return gNEnBins; }

    template <typename Particle>
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(Particle& p, const corsika::particles::Code TargetId);

    template <typename Particle>
    corsika::units::si::GrammageType GetInteractionLength(Particle&);

    template <typename Projectile>
    corsika::process::EProcessReturn DoInteraction(Projectile&);

  private:
    TEnvironment const& fEnvironment;
    corsika::process::sibyll::Interaction& fHadronicInteraction;
    std::map<corsika::particles::Code, int> fTargetComponentsIndex;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
    static constexpr int gNSample =
        500; // number of samples in MC estimation of cross section
    static constexpr int gMaxNucleusAProjectile = 56;
    static constexpr int gNEnBins = 6;
    static constexpr int gMaxNFragments = 60;
    // energy limits defined by table used for cross section in signuc.f
    // 10**1 GeV to 10**6 GeV
    static constexpr corsika::units::si::HEPEnergyType gMinEnergyPerNucleonCoM =
        10. * 1e9 * corsika::units::si::electronvolt;
    static constexpr corsika::units::si::HEPEnergyType gMaxEnergyPerNucleonCoM =
        1.e6 * 1e9 * corsika::units::si::electronvolt;
  };

} // namespace corsika::process::sibyll

#endif
