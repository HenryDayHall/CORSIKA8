#pragma once

/**
 * \function setup_stack
 *
 * standard stack setup for unit tests.
 * \todo This can be moved to "test" directory, when available.
 */

namespace corsika::setup::testing {

  inline std::tuple<std::unique_ptr<setup::Stack>, std::unique_ptr<setup::StackView>>
  setup_stack(Code vProjectileType, int vA, int vZ, HEPEnergyType vMomentum,
              setup::Environment::BaseNodeType* const vNodePtr,
              CoordinateSystem const& cs) {

    auto stack = std::make_unique<setup::Stack>();

    Point const origin(cs, {0_m, 0_m, 0_m});
    MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

    if (vProjectileType == Code::Nucleus) {
      auto constexpr mN = constants::nucleonMass;
      HEPEnergyType const E0 = sqrt(static_pow<2>(mN * vA) + pLab.squaredNorm());
      auto particle = stack->AddParticle(
          std::make_tuple(Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ));
      particle.SetNode(vNodePtr);
      return std::make_tuple(std::move(stack),
                             std::make_unique<setup::StackView>(particle));
    } else { // not a nucleus
      HEPEnergyType const E0 =
          sqrt(static_pow<2>(GetMass(vProjectileType)) + pLab.squaredNorm());
      auto particle =
          stack->AddParticle(std::make_tuple(vProjectileType, E0, pLab, origin, 0_ns));
      particle.SetNode(vNodePtr);
      return std::make_tuple(std::move(stack),
                             std::make_unique<setup::StackView>(particle));
    }
  }

} // namespace corsika::setup::testing
