/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_HadronicElasticInteraction_h
#define _include_HadronicElasticInteraction_h

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>

#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::HadronicElasticModel {
  /**
   * A simple model for elastic hadronic interactions based on the formulas
   * in Gaisser, Engel, Resconi, Cosmic Rays and Particle Physics (Cambridge Univ. Press,
   * 2016), section 4.2 and Donnachie, Landshoff, Phys. Lett. B 296, 227 (1992)
   *
   * Currently only \f$p\f$ projectiles are supported and cross-sections are assumed to be
   * \f$pp\f$-like even for nuclei.
   */
  class HadronicElasticInteraction
      : public corsika::process::InteractionProcess<HadronicElasticInteraction> {
  private:
    corsika::units::si::CrossSectionType const fX, fY;

    static double constexpr gfEpsilon = 0.0808;
    static double constexpr gfEta = 0.4525;
    // Froissart-Martin is not violated up for sqrt s < 10^32 eV with these values [DL].

    using SquaredHEPEnergyType = decltype(corsika::units::si::HEPEnergyType() *
                                          corsika::units::si::HEPEnergyType());

    using eV2 = decltype(units::si::square(units::si::electronvolt));
    using inveV2 = decltype(1 / units::si::square(units::si::electronvolt));

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream(
            "HadronicElasticModel");

    inveV2 B(eV2 s) const;
    corsika::units::si::CrossSectionType CrossSection(SquaredHEPEnergyType s) const;

  public:
    HadronicElasticInteraction( // x & y values taken from DL for pp collisions
        units::si::CrossSectionType x = 0.0217 * units::si::barn,
        units::si::CrossSectionType y = 0.05608 * units::si::barn);
    void Init();

    template <typename Particle>
    corsika::units::si::GrammageType GetInteractionLength(Particle const& p);

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);
  };

} // namespace corsika::process::HadronicElasticModel

#endif
