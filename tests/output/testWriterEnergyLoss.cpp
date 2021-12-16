/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/modules/writers/EnergyLossWriter.hpp>

#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/ShowerAxis.hpp>

#include <corsika/framework/geometry/StraightTrajectory.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <corsika/framework/core/Logging.hpp>

using namespace corsika;

const auto density = 1_kg / (1_m * 1_m * 1_m);

auto setupEnvironment(Code vTargetCode) {
  // setup environment, geometry
  auto env = std::make_unique<Environment<IMediumModel>>();
  auto& universe = *(env->getUniverse());
  const CoordinateSystemPtr& cs = env->getCoordinateSystem();

  auto theMedium = Environment<IMediumModel>::createNode<Sphere>(
      Point{cs, 0_m, 0_m, 0_m}, 1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = HomogeneousMedium<IMediumModel>;
  theMedium->setModelProperties<MyHomogeneousModel>(
      density, NuclearComposition({vTargetCode}, {1.}));

  auto const* nodePtr = theMedium.get();
  universe.addChild(std::move(theMedium));

  return std::make_tuple(std::move(env), &cs, nodePtr);
}

class TestEnergyLoss : public corsika::EnergyLossWriter<> {
public:
  TestEnergyLoss(corsika::ShowerAxis const& axis)
      : EnergyLossWriter(axis) {}

  YAML::Node getConfig() const { return YAML::Node(); }
};

TEST_CASE("EnergyLossWriter") {

  logging::set_level(logging::level::info);

  auto [env, csPtr, nodePtr] = setupEnvironment(Code::Nitrogen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  auto const observationHeight = 0_km;
  auto const injectionHeight = 10_km;
  auto const t = -observationHeight + injectionHeight;
  Point const showerCore{cs, 0_m, 0_m, observationHeight};
  Point const injectionPos = showerCore + Vector<dimensionless_d>{cs, {0, 0, 1}} * t;

  ShowerAxis const showerAxis{injectionPos, (showerCore - injectionPos), *env,
                              true, // -> throw exceptions
                              20};  // -> number of bins

  // preparation
  if (boost::filesystem::exists("./output_dir_eloss")) {
    boost::filesystem::remove_all("./output_dir_eloss");
  }
  boost::filesystem::create_directory("./output_dir_eloss");

  TestEnergyLoss test(showerAxis);
  test.startOfLibrary("./output_dir_eloss");
  test.startOfShower(0);

  CHECK(test.getEnergyLost() / 1_GeV == Approx(0));
  test.write(100_g / square(1_cm), 110_g / square(1_cm), Code::Photon, 100_GeV);
  CHECK(test.getEnergyLost() / 1_GeV == Approx(100));
  test.write(Point(cs, {0_m, 0_m, observationHeight + 10_km}), Code::Proton, 100_GeV);
  CHECK(test.getEnergyLost() / 1_GeV == Approx(200));

  // generate straight simple track
  CoordinateSystemPtr rootCS = get_root_CoordinateSystem();
  Point r0(rootCS, {0_m, 0_m, 5_km});
  SpeedType const V0 = constants::c;
  VelocityVector v0(rootCS, {V0, 0_m / second, 0_m / second});
  Line const line(r0, v0);
  auto const time = 10_ns;
  StraightTrajectory track(line, time);
  // test write
  test.write(track, Code::Proton, 100_GeV);

  // incompatible binning
  CHECK_THROWS(
      test.write(100_g / square(1_cm), 120_g / square(1_cm), Code::Photon, 100_GeV));

  test.endOfShower(0);
  test.endOfLibrary();

  CHECK(boost::filesystem::exists("./output_dir_eloss/dEdX.parquet"));
}
