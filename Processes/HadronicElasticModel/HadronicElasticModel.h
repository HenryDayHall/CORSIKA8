/**
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

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

#include <iomanip>

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
    static double constexpr fEpsilon = 0.0808;
    static double constexpr fEta = 0.4525;

    using SquaredHEPEnergyType = decltype(corsika::units::si::HEPEnergyType() *
                                          corsika::units::si::HEPEnergyType());

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream(
            "HadronicElasticModel");
    corsika::environment::Environment const& fEnvironment;

    auto B(corsika::units::si::HEPEnergyType sqrtS) const {
      using namespace corsika::units::si;
      auto constexpr a =
          2.5 / (1_GeV * 1_GeV); // read off by eye from Gaisser, Engel, Resconi, fig. 4.9
      auto constexpr b = 10 / (1_GeV * 1_GeV);
      return a * std::log10(sqrtS / 10_GeV) + b;
    }

    corsika::units::si::CrossSectionType CrossSection(SquaredHEPEnergyType s) const {
      using namespace corsika::units::si;
      // according to Gaisser, Engel, Resconi, eq. (4.41)
      // assuming every target behaves like a proton
      CrossSectionType const sigmaTot = fX * pow(s * (1 / (1_GeV * 1_GeV)), fEpsilon) +
                                        fY * pow(s * (1 / (1_GeV * 1_GeV)), -fEta);

      // we ignore rho because rho^2 is just ~2 %
      auto const sigmaElastic =
          sigmaTot * sigmaTot /
          (16 * M_PI * ConvertHEPToSI<CrossSectionType::dimension_type>(B(sqrt(s))));
      return sigmaElastic;
    }

  public:
    HadronicElasticInteraction(corsika::environment::Environment const&,
                               units::si::CrossSectionType x = 2.17e-5 * units::si::barn,
                               units::si::CrossSectionType y = 5.608e-5 *
                                                               units::si::barn);
    void Init();

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle const& p, Track&) {
      using namespace corsika::units::si;
      if (p.GetPID() == corsika::particles::Code::Proton) {
        auto const* currentNode =
            fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
        auto const& mediumComposition =
            currentNode->GetModelProperties().GetNuclearComposition();

        auto const& components = mediumComposition.GetComponents();
        auto const& fractions = mediumComposition.GetFractions();

        auto const projectileMomentum = p.GetMomentum();
        auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
        auto const projectileEnergy = p.GetEnergy();

        auto const avgCrossSection = [&]() {
          CrossSectionType avgCrossSection = 0_barn;

          for (size_t i = 0; i < fractions.size(); ++i) {
            auto const targetMass = corsika::particles::GetMass(components[i]);
            auto const s = detail::static_pow<2>(projectileEnergy + targetMass) -
                           projectileMomentumSquaredNorm;
            avgCrossSection += CrossSection(s) * fractions[i];
          }

          std::cout << "avgCrossSection: " << avgCrossSection / 1_mbarn << " mb"
                    << std::endl;

          return avgCrossSection;
        }();

        auto const avgTargetMassNumber = mediumComposition.GetAverageMassNumber();

        GrammageType const interactionLength =
            avgTargetMassNumber * corsika::units::constants::u / avgCrossSection;

        return interactionLength;
      } else {
        return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
    }

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack&) {
      if (p.GetPID() != corsika::particles::Code::Proton) {
        return process::EProcessReturn::eOk;
      }

      using namespace units::si;

      const auto* currentNode =
          fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
      const auto& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();
      const auto& components = mediumComposition.GetComponents();

      std::vector<units::si::CrossSectionType> cross_section_of_components(
          mediumComposition.GetComponents().size());

      auto const projectileMomentum = p.GetMomentum();
      auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
      auto const projectileEnergy = p.GetEnergy();

      for (size_t i = 0; i < components.size(); ++i) {
        auto const targetMass = corsika::particles::GetMass(components[i]);
        auto const s = units::si::detail::static_pow<2>(projectileEnergy + targetMass) -
                       projectileMomentumSquaredNorm;
        cross_section_of_components[i] = CrossSection(s);
      }

      const auto targetCode = currentNode->GetModelProperties().SampleTarget(
          cross_section_of_components, fRNG);

      auto const targetMass = corsika::particles::GetMass(targetCode);

      std::uniform_real_distribution phiDist(0., 2 * M_PI);

      utl::COMBoost const boost(projectileEnergy, projectileMomentum, targetMass);
      auto const [eProjectileCoM, pProjectileCoM] =
          boost.toCoM(projectileEnergy, projectileMomentum);
      auto const [eTargetCoM, pTargetCoM] = boost.toCoM(
          targetMass, geometry::Vector<units::si::hepmomentum_d>(
                          projectileMomentum.GetCoordinateSystem(), {0_eV, 0_eV, 0_eV}));

      auto const pProjectileCoMSqNorm = pProjectileCoM.squaredNorm();
      auto const pProjectileCoMNorm = sqrt(pProjectileCoMSqNorm);

      auto const sqrtS = eProjectileCoM + eTargetCoM;
      auto const s = units::si::detail::static_pow<2>(sqrtS);
      
      auto const B = this->B(sqrtS);
      std::cout << B << std::endl;

      corsika::random::ExponentialDistribution tDist(1 / B);
      auto const absT = [&]() {
        decltype(tDist(fRNG)) absT;
        auto const maxT = 4 * pProjectileCoMSqNorm;

        do {
          // |t| cannot become arbitrarily large, max. given by GEN eq. (4.16), so we just
          // throw again until we have an acceptable value note that the formula holds in
          // any frame despite of what is stated there
          absT = tDist(fRNG);
        } while (absT >= maxT);

        return absT;
      }();

      auto constexpr GeVsq = 1_GeV * 1_GeV;
      std::cout << "s = " << s / GeVsq << " GeV²; absT = " << absT / GeVsq
                << " GeV² (max./GeV² = " << 4 * projectileMomentumSquaredNorm / GeVsq
                << ')' << std::endl;

      auto const theta =
          2 * asin(sqrt(absT / (4 * pProjectileCoMSqNorm))); // would work for in any frame
      auto const phi = phiDist(fRNG);

      geometry::QuantityVector<units::si::hepmomentum_d> const scatteredMomentum{
          pProjectileCoMNorm * sin(theta) * cos(phi),
          pProjectileCoMNorm * sin(theta) * sin(phi), pProjectileCoMNorm * cos(theta)};
          
      auto const [eProjectileScatteredLab, pProjectileScatteredLab] =
          boost.fromCoM(eProjectileCoM, scatteredMomentum);
          
      std::cout << "xxx: " << pProjectileScatteredLab.squaredNorm() / p.GetMomentum().squaredNorm() << std::endl;

      p.SetMomentum(pProjectileScatteredLab);
      
      // alternatives: 1. do not touch energy
      //               2. take back-boosted energy
      //               3. recalculate energy from momentum
      
      //~ p.SetEnergy(sqrt(pProjectileScatteredLab.squaredNorm() +
                       //~ units::si::detail::static_pow<2>(corsika::particles::GetMass(p.GetPID()))));

      return process::EProcessReturn::eOk;
    }
  };
} // namespace corsika::process::HadronicElasticModel

#endif
