#ifndef _Framework_Cascade_testCascade_h
#define _Framework_Cascade_testCascade_h

#include <corsika/environment/Environment.h>
#include <corsika/setup/SetupStack.h>

using TestEnvironmentType = corsika::environment::Environment<corsika::environment::IMediumModel>;

template <typename T>
using SetupGeometryDataInterface = GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface =
        corsika::stack::CombinedParticleInterface<corsika::setup::detail::ParticleDataStack::PIType,
                                                  SetupGeometryDataInterface, StackIter>;
using TestCascadeStack = corsika::stack::CombinedStack<typename corsika::setup::detail::ParticleDataStack::StackImpl, GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;

#endif
