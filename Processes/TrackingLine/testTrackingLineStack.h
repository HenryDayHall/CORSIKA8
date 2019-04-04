/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_process_trackinling_teststack_h_
#define _include_process_trackinling_teststack_h_

#include <corsika/environment/Environment.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

using TestEnvironmentType =
    corsika::environment::Environment<corsika::environment::Empty>;

template <typename T>
using SetupGeometryDataInterface = GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
    corsika::setup::detail::ParticleDataStack::PIType, SetupGeometryDataInterface,
    StackIter>;
using TestTrackingLineStack = corsika::stack::CombinedStack<
    typename corsika::setup::detail::ParticleDataStack::StackImpl,
    GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

#endif
