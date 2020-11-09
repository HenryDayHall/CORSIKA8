/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/tracking_bfield/Tracking.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>

#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
using corsika::setup::Trajectory;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::particles;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::geometry;
using namespace corsika::units::si;

template <typename T>
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

TEST_CASE("TrackingBField") {

  logging::SetLevel(logging::level::trace);

  const HEPEnergyType P0 = 10_GeV;

  auto PID = GENERATE(as<Code>{}, Code::MuPlus, Code::MuPlus, Code::Gamma);
  auto Bfield = GENERATE(as<MagneticFluxType>{}, 0_T, 50_uT, -50_uT);
  auto outer = GENERATE(as<bool>{}, true, false);

  SECTION(fmt::format("Tracking PID={}, Bfield={} uT, from outside={}", PID,
                      Bfield / 1_uT, outer)) {

    C8LOG_DEBUG(
        "********************\n                          TEST section PID={}, Bfield={} "
        "uT, outer (?)={}",
        PID, Bfield / 1_uT, outer);

    const int chargeNumber = GetChargeNumber(PID);
    LengthType radius = 10_m;
    int deflect = 0;
    if (chargeNumber != 0 and Bfield != 0_T) {
      deflect = -sgn(chargeNumber) * sgn(Bfield / 1_T); // direction of deflection
      LengthType const gyroradius =
	P0 * 1_V / (constants::c * abs(chargeNumber) * abs(Bfield) * 1_eV);
      radius = gyroradius;
    }

    auto [env, csPtr, worldPtr] = setup::testing::setupEnvironment(Code::Oxygen, Bfield);
    { [[maybe_unused]] const auto& env_dummy = env; }
    auto const& cs = *csPtr;

    tracking_bfield::Tracking tracking;
    using tracking_bfield::MagneticFieldVector;
    Point const center(cs, {0_m, 0_m, 0_m});
    auto target = setup::Environment::CreateNode<geometry::Sphere>(center, radius);

    using MyHomogeneousModel =
        environment::MediumPropertyModel<environment::UniformMagneticField<
            environment::HomogeneousMedium<setup::EnvironmentInterface>>>;

    MagneticFieldVector magneticfield(cs, 0_T, 0_T, Bfield);
    target->SetModelProperties<MyHomogeneousModel>(
        environment::Medium::AirDry1Atm, magneticfield, 1_g / (1_m * 1_m * 1_m),
        environment::NuclearComposition(std::vector<Code>{Code::Oxygen},
                                        std::vector<float>{1.}));
    auto* targetPtr = target.get();
    worldPtr->AddChild(std::move(target));

    auto [stack, viewPtr] = setup::testing::setupStack(PID, 0, 0, P0, targetPtr, cs);
    { [[maybe_unused]] auto& viewPtr_dum = viewPtr; }
    auto particle = stack->first();
    // Note: momentum in X-direction
    //       magnetic field in Z-direction
    //       put particle on x_start, 0, 0
    //       expect intersections somewere in +-y_start

    if (outer) {
      particle.SetNode(worldPtr); // set particle inside "target" volume
    } else {
      particle.SetNode(targetPtr); // set particle outside "target" volume
    }
    particle.SetPosition(Point(cs, -radius, 0_m, 0_m));

    auto [traj, nextVol] = tracking.GetTrack(particle);
    particle.SetNode(nextVol);
    particle.SetPosition(traj.GetPosition(1));
    particle.SetMomentum(traj.GetDirection(1) * particle.GetMomentum().norm());
    if (outer) {
      // now we know we are in target volume, depending on "outer"
      CHECK(traj.GetLength(1) == 0_m);
      CHECK(nextVol == targetPtr);
    }
    // move forward, until we leave target volume
    while (nextVol == targetPtr) {
      const auto [traj2, nextVol2] = tracking.GetTrack(particle);
      nextVol = nextVol2;
      particle.SetNode(nextVol);
      particle.SetPosition(traj2.GetPosition(1));
      particle.SetMomentum(traj2.GetDirection(1) * particle.GetMomentum().norm());
    }
    CHECK(nextVol == worldPtr);

    Point pointCheck(cs, (deflect == 0 ? radius : 0_m), (deflect * radius), 0_m);

    C8LOG_DEBUG("testTrackingLineStack: deflect={}, momentum={}, pos={}, pos_check={}",
                deflect, particle.GetMomentum().GetComponents(),
                particle.GetPosition().GetCoordinates(), pointCheck.GetCoordinates());

    CHECK((particle.GetPosition() - pointCheck).norm() / radius == Approx(0).margin(1e-3));
  }
}
