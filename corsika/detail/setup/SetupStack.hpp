#pragma once

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/framework/stack/node/GeometryNodeStackExtension.hpp>
#include <corsika/framework/stack/nuclear_extension/NuclearStackExtension.hpp>
#include <corsika/framework/stack/history/HistorySecondaryProducer.hpp>
#include <corsika/framework/stack/history/HistoryStackExtension.hpp>

#include <corsika/setup/SetupEnvironment.hpp>

namespace corsika {

  namespace setup::detail {

    // ------------------------------------------
    // add geometry node tracking data to stack:

    // the GeometryNode stack needs to know the type of geometry-nodes from the
    // environment:
    template <typename TStackIter>
    using SetupGeometryDataInterface =
        typename MakeGeometryDataInterface<TStackIter, setup::Environment>::type;

    // combine particle data stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithGeometryInterface =
        CombinedParticleInterface<nuclear_extension::ParticleDataStack::MPIType,
                                  SetupGeometryDataInterface, TStackIter>;

    using StackWithGeometry =
        CombinedStack<typename nuclear_extension::ParticleDataStack::StackImpl,
                      GeometryData<setup::Environment>, StackWithGeometryInterface>;

    // ------------------------------------------
    // Add [optional] history data to stack, too:

    // combine dummy stack with geometry information for tracking
    template <typename TStackIter>
    using StackWithHistoryInterface =
        CombinedParticleInterface<StackWithGeometry::MPIType, HistoryEventDataInterface,
                                  TStackIter>;

    using StackWithHistory = CombinedStack<typename StackWithGeometry::StackImpl,
                                           HistoryEventData, StackWithHistoryInterface>;

  } // namespace setup::detail

} // namespace corsika
