/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/stack/GeometryNodeStackExtension.hpp>
#include <corsika/stack/NuclearStackExtension.hpp>
#include <corsika/stack/WeightStackExtension.hpp>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>
#include <corsika/stack/history/HistoryStackExtension.hpp>

#include <corsika/setup/SetupEnvironment.hpp>

namespace corsika {

  namespace setup::detail {

    // ------------------------------------------
    // add geometry node tracking data to stack:

    // the GeometryNode stack needs to know the type of geometry-nodes from the
    // environment:
    template <typename TStackIter>
    using SetupGeometryDataInterface =
        typename node::MakeGeometryDataInterface<TStackIter, setup::Environment>::type;

    // combine particle data stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithGeometryInterface =
        CombinedParticleInterface<nuclear_stack::ParticleDataStack::pi_type,
                                  SetupGeometryDataInterface, TStackIter>;

    using StackWithGeometry = CombinedStack<
        typename nuclear_stack::ParticleDataStack::stack_implementation_type,
        node::GeometryData<setup::Environment>, StackWithGeometryInterface>;

    // ------------------------------------------
    // Add [optional] history data to stack, too:

    // combine dummy stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithHistoryInterface =
        CombinedParticleInterface<StackWithGeometry::pi_type,
                                  history::HistoryEventDataInterface, TStackIter>;

    using StackWithHistory =
        CombinedStack<typename StackWithGeometry::stack_implementation_type,
                      history::HistoryEventData, StackWithHistoryInterface>;

  } // namespace setup::detail

} // namespace corsika
