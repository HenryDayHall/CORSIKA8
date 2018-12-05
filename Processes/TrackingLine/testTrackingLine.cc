/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>

#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/geometry/Sphere.h>

#include <corsika/setup/SetupTrajectory.h>
using corsika::setup::Trajectory;

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::geometry;

#include <iostream>
using namespace std;
using namespace corsika::units::si;

struct DummyParticle {
    EnergyType fEnergy;
    Vector<momentum_d> fMomentum;
    Point fPosition;
    
    DummyParticle(EnergyType pEnergy, Vector<momentum_d> pMomentum, Point pPosition) : fEnergy(pEnergy), fMomentum(pMomentum), fPosition(pPosition) {}
    
    auto GetEnergy() const { return fEnergy; }
    auto GetMomentum() const { return fMomentum; }
    auto GetPosition() const { return fPosition; }
};

struct DummyStack {
    using ParticleType = DummyParticle;
};

TEST_CASE("TrackingLine") {
  corsika::environment::Environment env; // dummy environment
  auto const& cs = env.GetCoordinateSystem();
  
  tracking_line::TrackingLine<DummyStack> tracking(env);

  SECTION("intersection with sphere") {
    Point const origin(cs, {0_m, 0_m, 0_m});
    Point const center(cs, {0_m, 0_m, 10_m});
    Sphere const sphere(center, 1_m);
    Vector<corsika::units::si::SpeedType::dimension_type> v(cs, 0_m/second, 0_m/second, 1_m/second);
    Line line(origin, v);
    
    geometry::Trajectory<Line> traj(line, 12345_s);
    
    auto const opt = tracking.TimeOfIntersection(traj, Sphere(Point(cs, {0_m, 0_m, 10_m}), 1_m));
    REQUIRE(opt.has_value());
    
    auto [t1, t2] = opt.value();
    REQUIRE(t1 / 9_s == Approx(1));
    REQUIRE(t2 / 11_s == Approx(1));
    
    auto const optNoIntersection = tracking.TimeOfIntersection(traj, Sphere(Point(cs, {5_m, 0_m, 10_m}), 1_m));
    REQUIRE_FALSE(optNoIntersection.has_value());
  }
  
  SECTION("maximally possible propagation") {
      auto& universe = *(env.GetUniverse());
      
      //~ std::cout << env.GetUniverse().get() << std::endl;
      
      DummyParticle p(1_J, Vector<momentum_d>(cs, 0*kilogram*meter/second, 0*kilogram*meter/second, 1*kilogram*meter/second), Point(cs, 0_m, 0_m,0_m));
      
      auto const radius = 20_m;
      
      auto theMedium = corsika::environment::Environment::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m}, radius);      
      universe.AddChild(std::move(theMedium));
      
      Point const origin(cs, {0_m, 0_m, 0_m});
      Vector<corsika::units::si::SpeedType::dimension_type> v(cs, 0_m/second, 0_m/second, 1_m/second);
      Line line(origin, v);
      
      auto const traj = tracking.GetTrack(p);
      
      REQUIRE((traj.GetPosition(1.) - Point(cs, 0_m, 0_m, radius)).GetComponents(cs).norm().magnitude() == Approx(0).margin(1e-4));
  }
}
