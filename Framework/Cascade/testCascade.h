/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Framework_Cascade_testCascade_h
#define _Framework_Cascade_testCascade_h

#include <corsika/environment/Environment.h>
#include <corsika/setup/SetupStack.h>

using TestEnvironmentType =
    corsika::environment::Environment<corsika::environment::IMediumModel>;

template <typename T>
using SetupGeometryDataInterface = GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
    corsika::setup::detail::ParticleDataStack::PIType, SetupGeometryDataInterface,
    StackIter>;

using TestCascadeStack = corsika::stack::CombinedStack<
    typename corsika::setup::detail::ParticleDataStack::StackImpl,
    GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

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

#endif
