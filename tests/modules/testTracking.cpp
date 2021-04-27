/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Tracking.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>
#include <SetupTestTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

template <typename T>
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

/**
 \file testTracking.cpp

  This is the unified and commond unit test for all Tracking algorithms:

  - tracking_leapfrog_curved::Tracking
  - tracking_leapfrog_straight::Tracking
  - tracking_line::Tracking

 */

TEMPLATE_TEST_CASE("TrackingLeapfrog_Curved", "tracking",
                   tracking_leapfrog_curved::Tracking,
                   tracking_leapfrog_straight::Tracking, tracking_line::Tracking) {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  logging::set_level(logging::level::trace);

  const HEPEnergyType P0 = 10_GeV;

  auto PID = GENERATE(as<Code>{}, Code::MuPlus, Code::MuPlus, Code::Photon);
  // for algorithms that know magnetic deflections choose: +-50uT, 0uT
  // otherwise just 0uT
  auto Bfield = GENERATE(filter(
      []([[maybe_unused]] MagneticFluxType v) {
        if constexpr (std::is_same_v<TestType, tracking_line::Tracking>)
          return v == 0_uT;
        else
          return true;
      },
      values<MagneticFluxType>({50_uT, 0_uT, -50_uT})));
  // particle --> (world) --> | --> (target)
  // true: start inside "world" volume
  // false: start inside "target" volume
  auto outer = GENERATE(as<bool>{}, true, false);

  SECTION(fmt::format("Tracking PID={}, Bfield={} uT, from outside={}", PID,
                      Bfield / 1_uT, outer)) {

    CORSIKA_LOG_DEBUG(
        "********************\n                          TEST section PID={}, Bfield={} "
        "uT, outer (?)={}",
        PID, Bfield / 1_uT, outer);

    const int chargeNumber = get_charge_number(PID);
    LengthType radius = 10_m;
    int deflect = 0;
    if (chargeNumber != 0 and Bfield != 0_T) {
      deflect = -sgn(chargeNumber) * sgn(Bfield / 1_T); // direction of deflection
      LengthType const gyroradius =
          P0 * 1_V / (constants::c * abs(chargeNumber) * abs(Bfield) * 1_eV);
      radius = gyroradius;
    }

    auto [env, csPtr, worldPtr] =
        corsika::setup::testing::setup_environment(Code::Oxygen, Bfield);
    { [[maybe_unused]] const auto& env_dummy = env; }
    auto const& cs = *csPtr;

    TestType tracking;
    Point const center(cs, {0_m, 0_m, 0_m});
    auto target = setup::Environment::createNode<Sphere>(center, radius);

    using MyHomogeneousModel = MediumPropertyModel<
        UniformMagneticField<HomogeneousMedium<setup::EnvironmentInterface>>>;

    MagneticFieldVector magneticfield(cs, 0_T, 0_T, Bfield);
    target->setModelProperties<MyHomogeneousModel>(
        Medium::AirDry1Atm, magneticfield, 1_g / (1_m * 1_m * 1_m),
        NuclearComposition(std::vector<Code>{Code::Oxygen}, std::vector<float>{1.}));
    auto* targetPtr = target.get();
    worldPtr->addChild(std::move(target));

    auto [stack, viewPtr] = setup::testing::setup_stack(PID, 0, 0, P0, targetPtr, cs);
    { [[maybe_unused]] auto& viewPtr_dum = viewPtr; }
    auto particle = stack->first();
    // Note: momentum in X-direction
    //       magnetic field in Z-direction
    //       put particle on x_start, 0, 0
    //       expect intersections somewere in +-y_start

    if (outer) {
      particle.setNode(worldPtr); // set particle inside "target" volume
    } else {
      particle.setNode(targetPtr); // set particle outside "target" volume
    }
    particle.setPosition(Point(cs, -radius, 0_m, 0_m));

    auto [traj, nextVol] = tracking.getTrack(particle);
    particle.setNode(nextVol);
    particle.setPosition(traj.getPosition(1));
    particle.setMomentum(traj.getDirection(1) * particle.getMomentum().getNorm());
    if (outer) {
      // now we know we are in target volume, depending on "outer"
      CHECK(traj.getLength(1) == 0_m);
      CHECK(nextVol == targetPtr);
    }
    // move forward, until we leave target volume
    while (nextVol == targetPtr) {
      const auto [traj2, nextVol2] = tracking.getTrack(particle);
      nextVol = nextVol2;
      particle.setNode(nextVol);
      particle.setPosition(traj2.getPosition(1));
      particle.setMomentum(traj2.getDirection(1) * particle.getMomentum().getNorm());
    }
    CHECK(nextVol == worldPtr);

    Point pointCheck(cs, (deflect == 0 ? radius : 0_m), (deflect * radius), 0_m);

    CORSIKA_LOG_DEBUG(
        "testTrackingLineStack: deflect={}, momentum={}, pos={}, pos_check={}", deflect,
        particle.getMomentum().getComponents(), particle.getPosition().getCoordinates(),
        pointCheck.getCoordinates());

    CHECK((particle.getPosition() - pointCheck).getNorm() / radius ==
          Approx(0).margin(1e-3));
  }
}
