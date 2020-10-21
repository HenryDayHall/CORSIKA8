/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/setup/SetupStack.hpp>

using TestEnvironmentType = corsika::Environment<corsika::Empty>;

template <typename T>
using SetupGeometryDataInterface = GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface =
    corsika::CombinedParticleInterface<corsika::setup::detail::ParticleDataStack::PIType,
                                       SetupGeometryDataInterface, StackIter>;
using TestTrackingLineStack =
    corsika::CombinedStack<typename corsika::setup::detail::ParticleDataStack::StackImpl,
                           GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;
