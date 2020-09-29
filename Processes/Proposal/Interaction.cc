/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::process::proposal {
  using namespace corsika::environment;
  using namespace corsika::units::si;

  template <>
  Interaction::Interaction(setup::SetupEnvironment const& _env,
                           particle_cut::ParticleCut& _cut)
      : ProposalProcessBase(_env, _cut) {}

  void Interaction::BuildCalculator(particles::Code code,
                                    environment::NuclearComposition const& comp) {
    auto c = cross[code](media.at(&comp), cut);
    auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(c);
    calc[std::make_pair(&comp, code)] = std::make_tuple(
        PROPOSAL::make_secondaries(inter_types, particle[code], media.at(&comp)),
        PROPOSAL::make_interaction(c, true));
  }

  template <>
  corsika::process::EProcessReturn Interaction::DoInteraction(
      setup::StackView::StackIterator& vP) {
    if (CanInteract(vP.GetPID())) {
      auto c = GetCalculator(vP, calc); // [CrossSections]
      std::uniform_real_distribution<double> distr(0., 1.);
      auto rates = get<INTERACTION>(c->second)->Rates(vP.GetEnergy() / 1_MeV);
      auto [type, comp_ptr, v] = get<INTERACTION>(c->second)->SampleLoss(
          vP.GetEnergy() / 1_MeV, rates, distr(fRNG));
      auto rnd = vector<double>(get<SECONDARIES>(c->second)->RequiredRandomNumbers(type));
      for (auto& it : rnd) it = distr(fRNG);
      auto point = PROPOSAL::Vector3D(vP.GetPosition().GetX() / 1_cm,
                                      vP.GetPosition().GetY() / 1_cm,
                                      vP.GetPosition().GetZ() / 1_cm);
      auto d = vP.GetDirection().GetComponents();
      auto direction = PROPOSAL::Vector3D(d.GetX().magnitude(), d.GetY().magnitude(),
                                          d.GetZ().magnitude());
      auto loss = make_tuple(static_cast<int>(type), point, direction,
                             v * vP.GetEnergy() / 1_MeV, 0.);
      auto sec = get<SECONDARIES>(c->second)->CalculateSecondaries(vP.GetEnergy() / 1_MeV,
                                                                   loss, *comp_ptr, rnd);
      for (auto& s : sec) {
        auto E = get<PROPOSAL::Loss::ENERGY>(s) * 1_MeV;
        auto vec = corsika::geometry::QuantityVector(
            get<PROPOSAL::Loss::DIRECTION>(s).GetX() * E,
            get<PROPOSAL::Loss::DIRECTION>(s).GetY() * E,
            get<PROPOSAL::Loss::DIRECTION>(s).GetZ() * E);
        auto p = corsika::stack::MomentumVector(
            corsika::geometry::RootCoordinateSystem::GetInstance()
                .GetRootCoordinateSystem(),
            vec);
        auto sec_code = corsika::particles::ConvertFromPDG(
            static_cast<particles::PDGCode>(get<PROPOSAL::Loss::TYPE>(s)));
        vP.AddSecondary(make_tuple(sec_code, E, p, vP.GetPosition(), vP.GetTime()));
      }
    }
    return process::EProcessReturn::eOk;
  }

  template <>
  corsika::units::si::GrammageType Interaction::GetInteractionLength(
      setup::Stack::StackIterator const& vP) {
    if (CanInteract(vP.GetPID())) {
      auto c = GetCalculator(vP, calc);
      return get<INTERACTION>(c->second)->MeanFreePath(vP.GetEnergy() / 1_MeV) * 1_g /
             (1_cm * 1_cm);
    }
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }
} // namespace corsika::process::proposal
