/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/setup/SetupStack.hpp>

using TestEnvironmentType =
    corsika::Environment<corsika::IMediumModel>;

template <typename T>
using SetupGeometryDataInterface =
    corsika::stack::node::GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface =
  corsika::CombinedParticleInterface<corsika::setup::detail::ParticleDataStack::PIType,
                                       SetupGeometryDataInterface, StackIter>;

using TestCascadeStack =
  corsika::CombinedStack<typename corsika::setup::detail::ParticleDataStack::StackImpl,
                           GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

/*
  See also Issue 161
*/
#if defined(__clang__)
using TestCascadeStackView = corsika::SecondaryView<typename TestCascadeStack::StackImpl,
                                                    StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using TestCascadeStackView = corsika::MakeView<TestCascadeStack>::type;
#endif
