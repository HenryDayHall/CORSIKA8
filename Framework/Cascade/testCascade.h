/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>

#include <corsika/stack/node/GeometryNodeStackExtension.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>
#include <corsika/stack/CombinedStack.h>

using TestEnvironmentType =
    corsika::environment::Environment<corsika::environment::IMediumModel>;

template <typename T>
using SetupGeometryDataInterface =
    corsika::stack::node::GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
    corsika::stack::nuclear_extension::ParticleDataStack::PIType,
    SetupGeometryDataInterface, StackIter>;

using TestCascadeStack = corsika::stack::CombinedStack<
    typename corsika::stack::nuclear_extension::ParticleDataStack::StackImpl,
    corsika::stack::node::GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

/*
  See also Issue 161
*/
#if defined(__clang__)
using TestCascadeStackView =
    corsika::stack::SecondaryView<typename TestCascadeStack::StackImpl,
                                  StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using TestCascadeStackView = corsika::stack::MakeView<TestCascadeStack>::type;
#endif
