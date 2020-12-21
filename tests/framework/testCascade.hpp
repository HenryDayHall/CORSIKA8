/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/stack/GeometryNodeStackExtension.hpp>
#include <corsika/stack/NuclearStackExtension.hpp>

using TestEnvironmentType = corsika::Environment<corsika::IMediumModel>;

template <typename T>
using SetupGeometryDataInterface =
    corsika::node::GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface =
    corsika::CombinedParticleInterface<corsika::nuclear_stack::ParticleDataStack::pi_type,
                                       SetupGeometryDataInterface, StackIter>;

using TestCascadeStack = corsika::CombinedStack<
    typename corsika::nuclear_stack::ParticleDataStack::stack_implementation_type,
    corsika::node::GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

/*
  See also Issue 161
*/
#if defined(__clang__)
using TestCascadeStackView = corsika::SecondaryView<typename TestCascadeStack::StackImpl,
                                                    StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using TestCascadeStackView = corsika::MakeView<TestCascadeStack>::type;
#endif
