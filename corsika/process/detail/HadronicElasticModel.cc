/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/hadronic_elastic_model/HadronicElasticModel.h>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <iomanip>
#include <iostream>
#include "../corsika/setup/SetupStack.hpp"

using namespace corsika;
using SetupParticle = corsika::Stack::ParticleType;

namespace corsika::HadronicElasticModel {

  void HadronicElasticInteraction::Init() {}

  HadronicElasticInteraction::HadronicElasticInteraction(units::si::CrossSectionType x,
                                                         units::si::CrossSectionType y)
      : fX(x)
      , fY(y) {}

  template <>
  units::si::GrammageType HadronicElasticInteraction::GetInteractionLength(
      SetupParticle const& p) {
    using namespace units::si;
    if (p.GetPID() == particles::Code::Proton) {
      auto const* currentNode = p.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();

      auto const& components = mediumComposition.GetComponents();
      auto const& fractions = mediumComposition.GetFractions();

      auto const projectileMomentum = p.GetMomentum();
      auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
      auto const projectileEnergy = p.GetEnergy();

HadronicElasticInteraction::HadronicElasticInteraction(units::si::CrossSectionType x,
                                                       units::si::CrossSectionType y)
    : fX(x)
    , fY(y) {}

template <>
units::si::GrammageType HadronicElasticInteraction::GetInteractionLength(
    SetupParticle const& p) {
  using namespace units::si;
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
        auto const s = units::static_pow<2>(projectileEnergy + targetMass) -
                       projectileMomentumSquaredNorm;
        avgCrossSection += CrossSection(s) * fractions[i];
      }

      std::cout << "avgCrossSection: " << avgCrossSection / 1_mb << " mb" << std::endl;

      return avgCrossSection;
    }();

    auto const avgTargetMassNumber = mediumComposition.GetAverageMassNumber();

    GrammageType const interactionLength =
        avgTargetMassNumber * units::constants::u / avgCrossSection;

    return interactionLength;
  } else {
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }
}

template <>
process::EProcessReturn HadronicElasticInteraction::DoInteraction(SetupView& view) {
  using namespace units::si;
  using namespace units::constants;

  auto p = view.GetProjectile();
  if (p.GetPID() != particles::Code::Proton) { return process::EProcessReturn::eOk; }

  const auto* currentNode = p.GetNode();
  const auto& composition = currentNode->GetModelProperties().GetNuclearComposition();
  const auto& components = composition.GetComponents();

  std::vector<units::si::CrossSectionType> cross_section_of_components(
      composition.GetComponents().size());

  auto const projectileMomentum = p.GetMomentum();
  auto const projectileMomentumSquaredNorm = projectileMomentum.squaredNorm();
  auto const projectileEnergy = p.GetEnergy();

  for (size_t i = 0; i < components.size(); ++i) {
    auto const targetMass = particles::GetMass(components[i]);
    auto const s = units::static_pow<2>(projectileEnergy + targetMass) -
                   projectileMomentumSquaredNorm;
    cross_section_of_components[i] = CrossSection(s);
  }

} // namespace corsika::HadronicElasticModel
