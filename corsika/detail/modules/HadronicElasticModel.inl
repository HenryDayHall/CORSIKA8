/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/HadronicElasticModel.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/utilities/COMBoost.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <iomanip>
#include <iostream>

namespace corsika::hadronic_elastic_model {

  void HadronicElasticInteraction::Init() {}

  HadronicElasticInteraction::HadronicElasticInteraction(CrossSectionType x,
                                                         CrossSectionType y)
      : fX(x)
      , fY(y) {}

  template <>
  GrammageType HadronicElasticInteraction::GetInteractionLength(SetupParticle const& p) {
    if (p.GetPID() == particles::Code::Proton) {
      auto const* currentNode = p.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();

      auto const& components = mediumComposition.GetComponents();
      auto const& fractions = mediumComposition.GetFractions();

      auto const projectileMomentum = p.GetMomentum();
      auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
      auto const projectileEnergy = p.GetEnergy();

      auto const avgCrossSection = [&]() {
        CrossSectionType avgCrossSection = 0_b;

        for (size_t i = 0; i < fractions.size(); ++i) {
          auto const targetMass = particles::GetMass(components[i]);
          auto const s = detail::static_pow<2>(projectileEnergy + targetMass) -
                         projectileMomentumSquaredNorm;
          avgCrossSection += CrossSection(s) * fractions[i];
        }

        std::cout << "avgCrossSection: " << avgCrossSection / 1_mb << " mb" << std::endl;

        return avgCrossSection;
      }();

      auto const avgTargetMassNumber = mediumComposition.GetAverageMassNumber();

      GrammageType const interactionLength =
          avgTargetMassNumber * constants::u / avgCrossSection;

      return interactionLength;
    } else {
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }
  }

  template <template TParticle>
  corsika::EProcessReturn HadronicElasticInteraction::DoInteraction(TParticle& p) {
    if (p.GetPID() != particles::Code::Proton) { return process::EProcessReturn::eOk; }

    const auto* currentNode = p.GetNode();
    const auto& composition = currentNode->GetModelProperties().GetNuclearComposition();
    const auto& components = composition.GetComponents();

    std::vector<CrossSectionType> cross_section_of_components(
        composition.GetComponents().size());

    auto const projectileMomentum = p.GetMomentum();
    auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
    auto const projectileEnergy = p.GetEnergy();

    for (size_t i = 0; i < components.size(); ++i) {
      auto const targetMass = corsika::GetMass(components[i]);
      auto const s = detail::static_pow<2>(projectileEnergy + targetMass) -
                     projectileMomentumSquaredNorm;
      cross_section_of_components[i] = CrossSection(s);
    }

    const auto targetCode = composition.SampleTarget(cross_section_of_components, fRNG);

    auto const targetMass = corsika::GetMass(targetCode);

    std::uniform_real_distribution phiDist(0., 2 * M_PI);

    geometry::FourVector const projectileLab(projectileEnergy, projectileMomentum);
    geometry::FourVector const targetLab(
        targetMass, corsika::Vector<hepmomentum_d>(
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
    auto const s = detail::static_pow<2>(sqrtS);

    auto const B = this->B(s);
    std::cout << B << std::endl;

    random::ExponentialDistribution tDist(1 / B);
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

    std::cout << "HadronicElasticInteraction: s = " << s * constants::invGeVsq
              << " GeV²; absT = " << absT * constants::invGeVsq << " GeV² (max./GeV² = "
              << 4 * constants::invGeVsq * projectileMomentumSquaredNorm << ')'
              << std::endl;

    auto const theta = 2 * asin(sqrt(absT / (4 * pProjectileCoMSqNorm)));
    auto const phi = phiDist(fRNG);

    auto const projectileScatteredLab =
        boost.fromCoM(corsika::FourVector<HEPEnergyType, corsika::Vector<hepmomentum_d>>(
            eProjectileCoM,
            corsika::Vector<hepmomentum_d>(projectileMomentum.GetCoordinateSystem(),
                                           {pProjectileCoMNorm * sin(theta) * cos(phi),
                                            pProjectileCoMNorm * sin(theta) * sin(phi),
                                            pProjectileCoMNorm * cos(theta)})));

    p.SetMomentum(projectileScatteredLab.GetSpaceLikeComponents());
    p.SetEnergy(
        sqrt(projectileScatteredLab.GetSpaceLikeComponents().squaredNorm() +
             detail::static_pow<2>(particles::GetMass(
                 p.GetPID())))); // Don't use energy from boost. It can be smaller than
                                 // the momentum due to limited numerical accuracy.

    return process::EProcessReturn::eOk;
  }

  HadronicElasticInteraction::inveV2 HadronicElasticInteraction::B(eV2 s) const {
    auto constexpr b_p = 2.3;
    auto const result =
        (2 * b_p + 2 * b_p + 4 * pow(s * constants::invGeVsq, gfEpsilon) - 4.2) *
        constants::invGeVsq;
    std::cout << "B(" << s << ") = " << result / invGeVsq << " GeV¯²" << std::endl;
    return result;
  }

  CrossSectionType HadronicElasticInteraction::CrossSection(
      SquaredHEPEnergyType s) const {
    // assuming every target behaves like a proton, fX and fY are universal
    CrossSectionType const sigmaTotal = fX * pow(s * constants::invGeVsq, gfEpsilon) +
                                        fY * pow(s * constants::invGeVsq, -gfEta);

    // according to Schuler & Sjöstrand, PRD 49, 2257 (1994)
    // (we ignore rho because rho^2 is just ~2 %)
    auto const sigmaElastic =
        detail::static_pow<2>(sigmaTotal) /
        (16 * constants::pi * ConvertHEPToSI<CrossSectionType::dimension_type>(B(s)));

    std::cout << "HEM sigmaTot = " << sigmaTotal / 1_mb << " mb" << std::endl;
    std::cout << "HEM sigmaElastic = " << sigmaElastic / 1_mb << " mb" << std::endl;
    return sigmaElastic;
  }

} // namespace corsika::hadronic_elastic_model
