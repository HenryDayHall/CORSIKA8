/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/tracking_line/TrackingLine.hpp>

#include <testTrackingLineStack.hpp> // test-build, and include file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <corsika/media/Environment.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/setup/SetupTrajectory.hpp>
using corsika::setup::Trajectory;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::units;

#include <iostream>
using namespace std;
using namespace corsika::units::si;

TEST_CASE("TrackingLine") {
  corsika::Environment<corsika::Empty> env; // dummy environment
  auto const& cs = env.GetCoordinateSystem();

  tracking_line::TrackingLine tracking;

  SECTION("intersection with sphere") {
    Point const origin(cs, {0_m, 0_m, -5_m});
    Point const center(cs, {0_m, 0_m, 10_m});
    Sphere const sphere(center, 1_m);
    Vector<corsika::units::si::SpeedType::dimension_type> v(cs, 0_m / second,
                                                            0_m / second, 1_m / second);
    Line line(origin, v);

    setup::Trajectory traj(line, 12345_s);

    auto const opt =
        tracking_line::TimeOfIntersection(traj, Sphere(Point(cs, {0_m, 0_m, 10_m}), 1_m));
    REQUIRE(opt.has_value());

    auto [t1, t2] = opt.value();
    REQUIRE(t1 / 14_s == Approx(1));
    REQUIRE(t2 / 16_s == Approx(1));

    auto const optNoIntersection =
        tracking_line::TimeOfIntersection(traj, Sphere(Point(cs, {5_m, 0_m, 10_m}), 1_m));
    REQUIRE_FALSE(optNoIntersection.has_value());
  }

  SECTION("maximally possible propagation") {
    auto& universe = *(env.GetUniverse());

    auto const radius = 20_m;

    auto theMedium = corsika::Environment<corsika::Empty>::CreateNode<Sphere>(
        Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m}, radius);
    auto const* theMediumPtr = theMedium.get();
    universe.AddChild(std::move(theMedium));

    TestTrackingLineStack stack;
    stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType,
                   corsika::MomentumVector, corsika::Point, units::si::TimeType>{
            corsika::Code::MuPlus,
            1_GeV,
            {cs, {0_GeV, 0_GeV, 1_GeV}},
            {cs, {0_m, 0_m, 0_km}},
            0_ns});
    auto p = stack.GetNextParticle();
    p.SetNode(theMediumPtr);

    Point const origin(cs, {0_m, 0_m, 0_m});
    Vector<corsika::units::si::SpeedType::dimension_type> v(cs, 0_m / second,
                                                            0_m / second, 1_m / second);
    Line line(origin, v);

    auto const [traj, geomMaxLength, nextVol] = tracking.GetTrack(p);
    [[maybe_unused]] auto dummy_geomMaxLength = geomMaxLength;
    [[maybe_unused]] auto dummy_nextVol = nextVol;

    REQUIRE((traj.GetPosition(1.) - Point(cs, 0_m, 0_m, radius))
                .GetComponents(cs)
                .norm()
                .magnitude() == Approx(0).margin(1e-4));
  }
}
