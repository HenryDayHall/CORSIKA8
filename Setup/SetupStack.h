/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/history/HistoryStackExtension.h>
#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/node/GeometryNodeStackExtension.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

#include <corsika/setup/SetupEnvironment.h>

namespace corsika::setup {

  namespace detail {

    // the GeometryNode stack needs to know the type of geometry-nodes from the
    // environment:
    template <typename TStackIter>
    using SetupGeometryDataInterface =
        typename stack::node::MakeGeometryDataInterface<TStackIter,
                                                        setup::SetupEnvironment>::type;

    /*
    template <typename TStackIter>
    using SetupGeometryHistoryDataInterface =
      typename stack::node::MakeHistoryDataInterface<TStackIter,
                                    ...event... >::type;
    */

    // combine particle data stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
        stack::nuclear_extension::ParticleDataStack::PIType, SetupGeometryDataInterface,
        TStackIter>;

    using StackWithGeometry = corsika::stack::CombinedStack<
        typename corsika::stack::nuclear_extension::ParticleDataStack::StackImpl,
        corsika::stack::node::GeometryData<setup::SetupEnvironment>,
        StackWithGeometryInterface>;

  } // namespace detail

  // this is the FINAL stack we use in C8:
  using Stack = detail::StackWithGeometry;

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
  using StackView =
      corsika::stack::SecondaryView<typename corsika::setup::Stack::StackImpl,
                                    // CHECK with CLANG: corsika::setup::Stack::PIType>;
                                    corsika::setup::detail::StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
  using StackView = corsika::stack::MakeView<corsika::setup::Stack>::type;
#endif

} // namespace corsika::setup
