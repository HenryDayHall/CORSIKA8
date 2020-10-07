/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/node/GeometryNodeStackExtension.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>
#include <corsika/stack/history/HistoryStackExtension.hpp>

#include <corsika/setup/SetupEnvironment.h>

namespace corsika::setup {

  namespace detail {

    // ------------------------------------------
    // add geometry node tracking data to stack:

    // the GeometryNode stack needs to know the type of geometry-nodes from the
    // environment:
    template <typename TStackIter>
    using SetupGeometryDataInterface =
        typename stack::node::MakeGeometryDataInterface<TStackIter,
                                                        setup::SetupEnvironment>::type;

    // combine particle data stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
        stack::nuclear_extension::ParticleDataStack::MPIType, SetupGeometryDataInterface,
        TStackIter>;

    using StackWithGeometry = corsika::stack::CombinedStack<
        typename corsika::stack::nuclear_extension::ParticleDataStack::StackImpl,
        corsika::stack::node::GeometryData<setup::SetupEnvironment>,
        StackWithGeometryInterface>;

    // ------------------------------------------
    // Add [optional] history data to stack, too:

    // combine dummy stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithHistoryInterface = corsika::stack::CombinedParticleInterface<
        StackWithGeometry::MPIType, history::HistoryEventDataInterface, TStackIter>;

    using StackWithHistory =
        corsika::stack::CombinedStack<typename StackWithGeometry::StackImpl,
                                      history::HistoryEventData,
                                      StackWithHistoryInterface>;

  } // namespace detail

  // ---------------------------------------
  // this is the FINAL stack we use in C8:

#ifdef WITH_HISTORY

  /*
   * the version with history
   */
  using Stack = detail::StackWithHistory;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = corsika::history::HistorySecondaryProducer<T1, M2>;

  namespace detail {
    /*
      See Issue 161

      unfortunately clang does not support this in the same way (yet) as
      gcc, so we have to distinguish here. If clang cataches up, we
      could remove the clang branch here and also in
      corsika::Cascade. The gcc code is much more generic and
      universal. If we could do the gcc version, we won't had to define
      StackView globally, we could do it with MakeView whereever it is
      actually needed. Keep an eye on this!
    */
#if defined(__clang__)
    using TheStackView = corsika::stack::SecondaryView<
        typename corsika::setup::Stack::StackImpl,
        // CHECK with CLANG: corsika::setup::Stack::MPIType>;
        corsika::setup::detail::StackWithHistoryInterface, StackViewProducer>;
#elif defined(__GNUC__) || defined(__GNUG__)
    using TheStackView =
        corsika::stack::MakeView<corsika::setup::Stack, StackViewProducer>::type;
#endif
  } // namespace detail

#else // WITH_HISTORY

  /*
   * the version without history
   */
  using Stack = detail::StackWithGeometry;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = corsika::stack::DefaultSecondaryProducer<T1, M2>;

  namespace detail {
    /*
      See Issue 161

      unfortunately clang does not support this in the same way (yet) as
      gcc, so we have to distinguish here. If clang cataches up, we
      could remove the clang branch here and also in
      corsika::Cascade. The gcc code is much more generic and
      universal. If we could do the gcc version, we won't had to define
      StackView globally, we could do it with MakeView whereever it is
      actually needed. Keep an eye on this!
    */
#if defined(__clang__)
    using TheStackView =
        corsika::stack::SecondaryView<typename corsika::setup::Stack::StackImpl,
                                      // CHECK with CLANG:
                                      // corsika::setup::Stack::MPIType>;
                                      corsika::setup::detail::StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
    using TheStackView = corsika::stack::MakeView<corsika::setup::Stack>::type;
#endif
  } // namespace detail

#endif

  // ---------------------------------------
  // this is the FINAL stackitertor (particle type) we use in C8:

  using StackView = detail::TheStackView;

} // namespace corsika::setup

namespace corsika::setup::testing {

  /**
   * standard setup for unit tests. This can be moved to "test"
   * directory, when available.
   */
  auto setupStack(particles::Code vProjectileType, int vA, int vZ,
                  units::si::HEPEnergyType vMomentum,
                  setup::Environment::BaseNodeType* vNodePtr,
                  geometry::CoordinateSystem const& cs) {

    using namespace corsika;
    using namespace corsika::units::si;

    auto stack = std::make_unique<setup::Stack>();
    auto constexpr mN = corsika::units::constants::nucleonMass;

    geometry::Point const origin(cs, {0_m, 0_m, 0_m});
    corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});
    
    if (vProjectileType == particles::Code::Nucleus) {
      HEPEnergyType const E0 = sqrt(units::static_pow<2>(mN * vA) + pLab.squaredNorm());
      auto particle = stack->AddParticle(
          std::make_tuple(particles::Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ));
    } else { // not a nucleus
      HEPEnergyType const E0 = sqrt(
          units::static_pow<2>(particles::GetMass(vProjectileType)) + pLab.squaredNorm());
      auto particle = stack->AddParticle(
          std::tuple<particles::Code, units::si::HEPEnergyType,
                     corsika::stack::MomentumVector, geometry::Point,
                     units::si::TimeType>{vProjectileType, E0, pLab, origin, 0_ns});
    }

    particle.SetNode(vNodePtr);
    return std::make_tuple(
        std::move(stack),
        std::make_unique<decltype(corsika::stack::SecondaryView(particle))>(particle));
  }

} // namespace corsika::setup::testing
