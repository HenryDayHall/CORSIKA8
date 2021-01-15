/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/setup/SetupStack.hpp>

/**
 * \file SetupTestStack
 *
 * standard stack setup for unit tests.
 **/

namespace corsika::setup::testing {

  /**
   * \function setup_stack
   *
   * standard stack setup for unit tests.
   *
   *
   *
   *
   * \return a tuple with element 0 being a Stack object filled with
   * one particle, and element 1 the StackView on it.
   **/

  inline std::tuple<std::unique_ptr<setup::Stack>, std::unique_ptr<setup::StackView>>
  setup_stack(Code vProjectileType, int vA, int vZ, HEPEnergyType vMomentum,
              setup::Environment::BaseNodeType* const vNodePtr,
              CoordinateSystemPtr const& cs) {

    auto stack = std::make_unique<setup::Stack>();

    Point const origin(cs, {0_m, 0_m, 0_m});
    MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

    if (vProjectileType == Code::Nucleus) {
      auto constexpr mN = constants::nucleonMass;
      HEPEnergyType const E0 = sqrt(static_pow<2>(mN * vA) + pLab.getSquaredNorm());
      auto particle = stack->addParticle(
          std::make_tuple(Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ));
      particle.setNode(vNodePtr);
      return std::make_tuple(std::move(stack),
                             std::make_unique<setup::StackView>(particle));
    } else { // not a nucleus
      HEPEnergyType const E0 =
          sqrt(static_pow<2>(get_mass(vProjectileType)) + pLab.getSquaredNorm());
      auto particle =
          stack->addParticle(std::make_tuple(vProjectileType, E0, pLab, origin, 0_ns));
      particle.setNode(vNodePtr);
      return std::make_tuple(std::move(stack),
                             std::make_unique<setup::StackView>(particle));
    }
  }

} // namespace corsika::setup::testing
