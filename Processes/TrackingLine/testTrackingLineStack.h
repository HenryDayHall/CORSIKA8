/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/node/GeometryNodeStackExtension.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

#include <corsika/units/PhysicalUnits.h>


class TestMagneticField {
    using MagneticFieldVector =
        corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>;

public:
  MagneticFieldVector GetMagneticField(corsika::geometry::Point const& p) const {
    using namespace corsika::units::si;
    return MagneticFieldVector(p.GetCoordinateSystem(), 0_T, 0_T, 1_T);
  }
};


using TestEnvironmentType =
    corsika::environment::Environment<TestMagneticField>;

template <typename T>
using SetupGeometryDataInterface =
    corsika::stack::node::GeometryDataInterface<T, TestEnvironmentType>;

// combine particle data stack with geometry information for tracking
template <typename StackIter>
using StackWithGeometryInterface = corsika::stack::CombinedParticleInterface<
    corsika::stack::nuclear_extension::ParticleDataStack::MPIType,
    SetupGeometryDataInterface, StackIter>;

using TestTrackingLineStack = corsika::stack::CombinedStack<
    typename corsika::stack::nuclear_extension::ParticleDataStack::StackImpl,
    corsika::stack::node::GeometryData<TestEnvironmentType>, StackWithGeometryInterface>;
