
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

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
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

    static double constexpr gfEpsilon = 0.0808;
    static double constexpr gfEta = 0.4525;
    // Froissart-Martin is not violated up for sqrt s < 10^32 eV with these values [DL].

    using SquaredHEPEnergyType = decltype(corsika::units::si::HEPEnergyType() *
                                          corsika::units::si::HEPEnergyType());

    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream(
            "HadronicElasticModel");
    corsika::environment::Environment const& fEnvironment;

    auto B(decltype(units::si::detail::static_pow<2>(units::si::electronvolt)) s) const {
      using namespace corsika::units::constants;
      auto constexpr b_p = 2.3;
      auto const result =
          (2 * b_p + 2 * b_p + 4 * pow(s * invGeVsq, gfEpsilon) - 4.2) * invGeVsq;
      std::cout << "B(" << s << ") = " << result / invGeVsq << " GeV¯²" << std::endl;
      return result;
    }

    corsika::units::si::CrossSectionType CrossSection(SquaredHEPEnergyType s) const {
      using namespace corsika::units::si;
      using namespace corsika::units::constants;
      // assuming every target behaves like a proton, fX and fY are universal
      CrossSectionType const sigmaTotal =
          fX * pow(s * invGeVsq, gfEpsilon) + fY * pow(s * invGeVsq, -gfEta);

      // according to Schuler & Sjöstrand, PRD 49, 2257 (1994)
      // (we ignore rho because rho^2 is just ~2 %)
      auto const sigmaElastic =
          units::si::detail::static_pow<2>(sigmaTotal) /
          (16 * M_PI * ConvertHEPToSI<CrossSectionType::dimension_type>(B(s)));

      std::cout << "HEM sigmaTot = " << sigmaTotal / 1_mbarn << " mb" << std::endl;
      std::cout << "HEM sigmaElastic = " << sigmaElastic / 1_mbarn << " mb" << std::endl;
      return sigmaElastic;
    }

  public:
    HadronicElasticInteraction(corsika::environment::Environment const&,
                               // x & y values taken from DL for pp collisions
                               units::si::CrossSectionType x = 0.0217 * units::si::barn,
                               units::si::CrossSectionType y = 0.05608 * units::si::barn);
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
      using namespace units::constants;

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

      geometry::FourVector const projectileLab(projectileEnergy, projectileMomentum);
      geometry::FourVector const targetLab(
          targetMass, geometry::Vector<units::si::hepmomentum_d>(
                          projectileMomentum.GetCoordinateSystem(), {0_eV, 0_eV, 0_eV}));
      utl::COMBoost const boost(projectileLab, targetMass);

      auto const projectileCoM = boost.toCoM(projectileLab);
      auto const targetCoM = boost.toCoM(targetLab);

      auto const pProjectileCoMSqNorm =
          projectileCoM.GetSpaceLikeComponents().squaredNorm();
      auto const pProjectileCoMNorm = sqrt(pProjectileCoMSqNorm);

      auto const eProjectileCoM = projectileCoM.GetTimeLikeComponent();
      auto const eTargetCoM = targetCoM.GetTimeLikeComponent();

      auto const sqrtS = eProjectileCoM + eTargetCoM;
      auto const s = units::si::detail::static_pow<2>(sqrtS);

      auto const B = this->B(s);
      std::cout << B << std::endl;

      corsika::random::ExponentialDistribution tDist(1 / B);
      auto const absT = [&]() {
        decltype(tDist(fRNG)) absT;
        auto const maxT = 4 * pProjectileCoMSqNorm;

        do {
          // |t| cannot become arbitrarily large, max. given by GER eq. (4.16), so we just
          // throw again until we have an acceptable value. Note that the formula holds in
          // any frame despite of what is stated in the book.
          absT = tDist(fRNG);
        } while (absT >= maxT);

        return absT;
      }();

      std::cout << "HadronicElasticInteraction: s = " << s * invGeVsq
                << " GeV²; absT = " << absT * invGeVsq
                << " GeV² (max./GeV² = " << 4 * invGeVsq * projectileMomentumSquaredNorm
                << ')' << std::endl;

      auto const theta = 2 * asin(sqrt(absT / (4 * pProjectileCoMSqNorm)));
      auto const phi = phiDist(fRNG);

      auto const projectileScatteredLab = boost.fromCoM(
          geometry::FourVector<HEPEnergyType, geometry::Vector<hepmomentum_d>>(
              eProjectileCoM,
              geometry::Vector<hepmomentum_d>(projectileMomentum.GetCoordinateSystem(),
                                              {pProjectileCoMNorm * sin(theta) * cos(phi),
                                               pProjectileCoMNorm * sin(theta) * sin(phi),
                                               pProjectileCoMNorm * cos(theta)})));

      p.SetMomentum(projectileScatteredLab.GetSpaceLikeComponents());
      p.SetEnergy(
          sqrt(projectileScatteredLab.GetSpaceLikeComponents().squaredNorm() +
               units::si::detail::static_pow<2>(corsika::particles::GetMass(
                   p.GetPID())))); // Don't use energy from boost. It can be smaller than
                                   // the momentum due to limited numerical accuracy.

      return process::EProcessReturn::eOk;
    }
  };
} // namespace corsika::process::HadronicElasticModel

#endif
