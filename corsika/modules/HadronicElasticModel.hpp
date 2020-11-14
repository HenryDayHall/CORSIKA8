/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>

#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika::hadronic_elastic_model {

  /**
   * A simple model for elastic hadronic interactions based on the formulas
   * in Gaisser, Engel, Resconi, Cosmic Rays and Particle Physics (Cambridge Univ. Press,
   * 2016), section 4.2 and Donnachie, Landshoff, Phys. Lett. B 296, 227 (1992)
   *
   * Currently only \f$p\f$ projectiles are supported and cross-sections are assumed to be
   * \f$pp\f$-like even for nuclei.
   */
  class HadronicElasticInteraction
      : public corsika::InteractionProcess<HadronicElasticInteraction> {
  private:
    CrossSectionType const fX, fY;

    static double constexpr gfEpsilon = 0.0808;
    static double constexpr gfEta = 0.4525;
    // Froissart-Martin is not violated up for sqrt s < 10^32 eV with these values [DL].

    using SquaredHEPEnergyType = decltype(HEPEnergyType() * HEPEnergyType());

    using eV2 = decltype(square(electronvolt));
    using inveV2 = decltype(1 / square(electronvolt));

    corsika::RNG& fRNG =
        corsika::RNGManager::getInstance().getRandomStream("HadronicElasticModel");

    inveV2 B(eV2 s) const;
    CrossSectionType CrossSection(SquaredHEPEnergyType s) const;

  public:
    HadronicElasticInteraction( // x & y values taken from DL for pp collisions
        CrossSectionType x = 0.0217 * barn, CrossSectionType y = 0.05608 * barn);
    void Init();

    template <typename Particle>
    GrammageType GetInteractionLength(Particle const& p);

    template <typename Particle>
    corsika::process::EProcessReturn DoInteraction(Particle&);
  };

} // namespace corsika::hadronic_elastic_model

#include <corsika/details/modules/HadronicElasticModel.inl>
