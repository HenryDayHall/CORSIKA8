/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/medium/MediumPropertyModel.hpp>
#include <corsika/media/density_and_composition/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/composition/NuclearComposition.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/ExponentialRefractiveIndex.hpp>
#include <corsika/media/refractivity/GladstoneDaleRefractiveIndex.hpp>
#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <SetupTestTrajectory.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <catch2/catch_all.hpp>

using namespace corsika;
using Catch::Approx;

template <typename TInterface>
using MyExtraEnv =
    ExponentialRefractiveIndex<media::MediumPropertyModel<media::UniformMagneticField<TInterface>>>;
template <typename TInterface2>
using MyExtraEnv2 =
    media::GladstoneDaleRefractiveIndex<media::MediumPropertyModel<media::UniformMagneticField<TInterface2>>>;

TEST_CASE("UniformRefractiveIndex w/ Homogeneous medium") {

  logging::set_level(logging::level::info);

  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  // set up our interface types
  using IModelInterface = media::IRefractiveIndexModel<media::IMediumModel>;
  using AtmModel = UniformRefractiveIndex<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition({Code::Proton}, {1.});

  // the refractive index that we use
  const double n{1.000327};

  // create the atmospheric model
  AtmModel medium(n, density, protonComposition);

  // and require that it is constant
  CHECK(n == medium.getRefractiveIndex(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(n == medium.getRefractiveIndex(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(n == medium.getRefractiveIndex(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(n == medium.getRefractiveIndex(Point(gCS, 100_km, 400_km, 350_km)));

  // a new refractive index
  const double n2{2.3472123};

  // update the refractive index of this atmospheric model
  medium.setRefractiveIndex(n2);

  // check that the returned refractive index is correct
  CHECK(n2 == medium.getRefractiveIndex(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(n2 == medium.getRefractiveIndex(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(n2 == medium.getRefractiveIndex(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(n2 == medium.getRefractiveIndex(Point(gCS, 100_km, 400_km, 350_km)));

  // define our axis vector
  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));

  // check the density and nuclear composition
  CHECK(density == medium.getMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.getNuclearComposition();

  SpeedType const speed = 1_m / second;

  // create a line of length 1 m
  Line const line(gOrigin, VelocityVector(gCS, {speed, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  LengthType const length = tEnd * speed;

  // and the associated trajectory
  setup::Trajectory const track =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.getIntegratedGrammage(track) / (density * length)) == Approx(1));
  CHECK((medium.getArclengthFromGrammage(track, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("ExponentialRefractiveIndex w/ Homogeneous medium") {

  logging::set_level(logging::level::info);

  // get a CS and a point
  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  // setup interface types
  using IModelInterface = media::IRefractiveIndexModel<media::IMediumModel>;
  using AtmModel = ExponentialRefractiveIndex<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition({Code::Proton}, {1.});

  // a new refractive index
  const double n0{1};
  const InverseLengthType lambda{6 / 1_m};

  // the center of the earth
  Point const center_{gCS, 0_m, 0_m, 0_m};
  // earth's radius
  LengthType const radius_{constants::EarthRadius::Mean};

  // create the atmospheric model and check refractive index
  AtmModel medium(n0, lambda, center_, constants::EarthRadius::Mean, density,
                  protonComposition);
  CHECK(n0 - medium.getRefractiveIndex(
                 Point(gCS, 0_m, 0_m, constants::EarthRadius::Mean)) ==
        Approx(0));

  // another refractive index
  const double n0_{1};
  const InverseLengthType lambda_{1 / 1_km};

  // distance from the center
  LengthType const dist_{4_km};

  // create the atmospheric model and check refractive index
  AtmModel medium_(n0_, lambda_, center_, dist_, density, protonComposition);
  CHECK(medium_.getRefractiveIndex(Point(gCS, 4_km, 3_km, 0_km)) == Approx(0.3678794412));

  // define axis vector
  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));

  // check the density and nuclear composition
  REQUIRE(density == medium.getMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.getNuclearComposition();
  REQUIRE(density == medium_.getMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium_.getNuclearComposition();

  SpeedType const velocity = 1_m / second;

  // the end time of our line
  TimeType const tEnd = 1_s;

  LengthType const length = tEnd * velocity;

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {velocity, 0_m / second, 0_m / second}));

  // and the associated trajectory
  setup::Trajectory const track =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  REQUIRE((medium.getIntegratedGrammage(track) / (density * length)) == Approx(1));
  REQUIRE((medium.getArclengthFromGrammage(track, density * 5_m) / 5_m) == Approx(1));
  REQUIRE((medium_.getIntegratedGrammage(track) / (density * length)) == Approx(1));
  REQUIRE((medium_.getArclengthFromGrammage(track, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("ExponentialRefractiveIndex w/ 5-layered atmosphere") {

  logging::set_level(logging::level::info);

  // get a CS
  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  // the center of the earth
  Point const center_{gCS, 0_m, 0_m, 0_m};

  // another refractive index
  const double n0{2};
  const InverseLengthType lambda{1 / 1_km};

  // a reference point to calculate the refractive index there
  Point const ref_{gCS, 0_m, 0_m, constants::EarthRadius::Mean};

  // setup a 5-layered environment
  using EnvironmentInterface =
      media::IRefractiveIndexModel<media::IMediumPropertyModel<media::IMagneticFieldModel<media::IMediumModel>>>;
  using EnvType = media::Environment<media::EnvironmentInterface>;
  EnvType env;

  media::create_5layer_atmosphere<media::EnvironmentInterface, MyExtraEnv>(
      env, media::AtmosphereId::LinsleyUSStd, center_, n0, lambda, center_,
      constants::EarthRadius::Mean, media::Medium::AirDry1Atm,
      MagneticFieldVector{gCS, 0_T, 50_uT, 0_T});

  // get the universe for this environment
  auto const* const universe{env.getUniverse().get()};
  auto const* node{universe->getContainingNode(ref_)};
  // get the refractive index
  auto const rIndex{node->getModelProperties().getRefractiveIndex(ref_)};

  CHECK(rIndex - n0 == Approx(0));
}

TEST_CASE("GladstoneDaleRefractiveIndex w/ Homogeneous medium") {

  logging::set_level(logging::level::info);

  // get a CS and a point
  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  // setup interface types
  using IModelInterface = media::IRefractiveIndexModel<media::IMediumModel>;
  using AtmModel = media::GladstoneDaleRefractiveIndex<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition({Code::Proton}, {1.});

  // the refractive index at sea level
  const double n0{1.000327};

  // a point at the surface of the earth
  Point const surface_{gCS, 0_m, 0_m, constants::EarthRadius::Mean};

  // a random point in the atmosphere
  Point const p1_{gCS, 1_km, 1_km, constants::EarthRadius::Mean + 10_km};

  // create the atmospheric model and check refractive index
  AtmModel medium(n0, surface_, density, protonComposition);

  CHECK(n0 - medium.getRefractiveIndex(surface_) == Approx(0));
  CHECK(n0 - medium.getRefractiveIndex(p1_) == Approx(0));
}

TEST_CASE("GladstoneDaleRefractiveIndex w/ 5-layered atmosphere") {

  logging::set_level(logging::level::info);

  // get a CS
  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  // the center of the earth
  Point const center_{gCS, 0_m, 0_m, 0_m};

  // a point at the surface of the earth
  Point const surface_{gCS, 0_m, 0_m, constants::EarthRadius::Mean};

  // the refractive index at sea level
  const double n0{1.000327};

  // a reference point to calculate the refractive index there
  Point const ref_{gCS, 0_km, 0_km, constants::EarthRadius::Mean + 10_km};

  // setup a 5-layered environment
  using EnvironmentInterface =
      media::IRefractiveIndexModel<media::IMediumPropertyModel<media::IMagneticFieldModel<media::IMediumModel>>>;
  using EnvType = media::Environment<media::EnvironmentInterface>;
  EnvType env;

  media::create_5layer_atmosphere<media::EnvironmentInterface, MyExtraEnv2>(
      env, media::AtmosphereId::LinsleyUSStd, center_, n0, surface_, media::Medium::AirDry1Atm,
      MagneticFieldVector{gCS, 0_T, 50_uT, 0_T});

  // get the universe for this environment
  auto const* const universe{env.getUniverse().get()};
  auto const* node{universe->getContainingNode(ref_)};
  // get the refractive index
  auto const rIndex1{node->getModelProperties().getRefractiveIndex(ref_)};
  auto const rIndex2{node->getModelProperties().getRefractiveIndex(surface_)};

  CHECK(rIndex1 - n0 == Approx(-0.0002591034));
  CHECK(rIndex2 - n0 == Approx(0));
}
