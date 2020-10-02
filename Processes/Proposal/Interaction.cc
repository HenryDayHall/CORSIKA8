/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
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

  template <>
  Interaction::Interaction(setup::SetupEnvironment const& _env,
                           corsika::units::si::HEPEnergyType _emCut)
      : ProposalProcessBase(_env, _emCut) {}

  void Interaction::BuildCalculator(particles::Code code,
                                    environment::NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto c = p_cross->second(media.at(&comp), emCut_);

    // Look which interactions take place and build the corresponding
    // interaction and secondarie builder. The interaction integral will
    // interpolated too and saved in the calc map by a key build out of a hash
    // of composed of the component and particle code.
    auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(c);
    calc[std::make_pair(&comp, code)] = std::make_tuple(
        PROPOSAL::make_secondaries(inter_types, particle[code], media.at(&comp)),
        PROPOSAL::make_interaction(c, true));
  }

  template <>
  corsika::process::EProcessReturn Interaction::DoInteraction(
      setup::StackView::StackIterator& vP) {
    using namespace corsika::units::si; // required for operator::_MeV

    if (CanInteract(vP.GetPID())) {
      // Get or build corresponding calculators
      auto c = GetCalculator(vP, calc);

      // Get the rates of the interaction types for every component.
      std::uniform_real_distribution<double> distr(0., 1.);

      // sample a interaction-type, loss and component
      auto rates = get<INTERACTION>(c->second)->Rates(vP.GetEnergy() / 1_MeV);
      auto [type, comp_ptr, v] = get<INTERACTION>(c->second)->SampleLoss(
          vP.GetEnergy() / 1_MeV, rates, distr(fRNG));

      // Read how much random numbers are required to calculate the secondaries.
      // Calculate the secondaries and deploy them on the corsika stack.
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
	std::cout << " proposal secondary: " << sec_code << " " << E/1_GeV << std::endl;
        vP.AddSecondary(make_tuple(sec_code, E, p, vP.GetPosition(), vP.GetTime()));
      }
    }
    return process::EProcessReturn::eOk;
  }

  template <>
  corsika::units::si::GrammageType Interaction::GetInteractionLength(
      setup::Stack::StackIterator const& vP) {
    using namespace corsika::units::si; // required for operator::_MeV

    if (CanInteract(vP.GetPID())) {
      auto c = GetCalculator(vP, calc);
      return get<INTERACTION>(c->second)->MeanFreePath(vP.GetEnergy() / 1_MeV) * 1_g /
             (1_cm * 1_cm);
    }
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }
} // namespace corsika::process::proposal
