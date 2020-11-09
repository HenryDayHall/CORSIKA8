/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/tracking_line/Tracking.h>
#include <testTrackingLineStack.h> // test-build, and include file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>
#include <corsika/geometry/Intersections.hpp>

#include <corsika/setup/SetupTrajectory.h>
using corsika::setup::Trajectory;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::geometry;

#include <iostream>
using namespace std;
using namespace corsika::units::si;

TEST_CASE("TrackingLine") {
}
