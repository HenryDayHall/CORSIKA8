/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/DensityFunction.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/IMediumPropertyModel.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/IRefractiveIndexModel.hpp>
#include <corsika/media/InhomogeneousMedium.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/LinearApproximationIntegrator.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>
#include <corsika/media/VolumeTreeNode.hpp>

#include <SetupTestTrajectory.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

Point const gOrigin(gCS, {0_m, 0_m, 0_m});

TEST_CASE("VolumeTree") {
  logging::set_level(logging::level::info);
  Environment<IEmpty> env;
  auto& universe = *(env.getUniverse());
  auto world = Environment<IEmpty>::createNode<Sphere>(Point{gCS, 0_m, 0_m, 0_m}, 150_km);
  // volume cut partly by "world"
  auto vol1 =
      Environment<IEmpty>::createNode<Sphere>(Point{gCS, 0_m, 0_m, 140_km}, 20_km);
  // partly overlap with "vol1"
  auto vol2 = Environment<IEmpty>::createNode<Sphere>(Point{gCS, 0_m, 0_m, 120_km}, 5_km);
  vol1->excludeOverlapWith(vol2);
  world->addChild(std::move(vol1));
  world->addChild(std::move(vol2));

  // in world
  CHECK(dynamic_cast<Sphere const&>(
            world->getContainingNode(Point(gCS, 0_m, 0_m, 0_m))->getVolume())
            .getRadius() == 150_km);
  // in vol1
  CHECK(dynamic_cast<Sphere const&>(
            world->getContainingNode(Point(gCS, 0_m, 0_m, 149_km))->getVolume())
            .getRadius() == 20_km);
  // outside world, in universe
  CHECK(world->getContainingNode(Point(gCS, 0_m, 151_km, 0_m)) == nullptr);
  // in vol2
  CHECK(dynamic_cast<Sphere const&>(
            world->getContainingNode(Point(gCS, 0_m, 0_km, 119_km))->getVolume())
            .getRadius() == 5_km);
  CHECK(dynamic_cast<Sphere const&>(
            world->getContainingNode(Point(gCS, 0_m, 0_km, 121_km))->getVolume())
            .getRadius() == 5_km);
  universe.addChild(std::move(world));
}

TEST_CASE("HomogeneousMedium") {

  logging::set_level(logging::level::info);

  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);

  CHECK_THROWS(NuclearComposition({Code::Proton}, {1.1}));
  CHECK_THROWS(NuclearComposition({Code::Proton}, {0.99}));
}

TEST_CASE("FlatExponential") {

  logging::set_level(logging::level::info);

  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / static_pow<3>(1_cm);
  FlatExponential<IMediumModel> const medium(gOrigin, axis, rho0, lambda,
                                             protonComposition);
  SpeedType const speed = 20_m / second;
  LengthType const length = 2_m;
  TimeType const tEnd = length / speed;

  SECTION("horizontal") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {speed, 0_m / second, 0_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);

    CHECK((medium.getIntegratedGrammage(trajectory) / (rho0 * length)) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, rho0 * length) / length) ==
          Approx(1));
  }

  SECTION("vertical") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, speed}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    GrammageType const exact = rho0 * lambda * (exp(length / lambda) - 1);

    CHECK((medium.getIntegratedGrammage(trajectory) / exact) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }

  SECTION("escape grammage") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {SpeedType::zero(), SpeedType::zero(), -speed}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    GrammageType const escapeGrammage = rho0 * lambda;

    CHECK(trajectory.getDirection(0).dot(axis).magnitude() < 0);
    CHECK(medium.getArclengthFromGrammage(trajectory, 1.2 * escapeGrammage) ==
          std::numeric_limits<typename GrammageType::value_type>::infinity() * 1_m);
  }

  SECTION("inclined") {
    Line const line(gOrigin,
                    Vector<SpeedType::dimension_type>(
                        gCS, {0_m / second, speed / sqrt(2.), speed / sqrt(2.)}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    double const cosTheta = M_SQRT1_2;
    GrammageType const exact =
        rho0 * lambda * (exp(cosTheta * length / lambda) - 1) / cosTheta;
    CHECK((medium.getIntegratedGrammage(trajectory) / exact) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }
}

TEST_CASE("SlidingPlanarExponential") {

  logging::set_level(logging::level::info);

  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / static_pow<3>(1_cm);
  auto const tEnd = 5_s;

  SlidingPlanarExponential<IMediumModel> const medium(gOrigin, rho0, lambda,
                                                      protonComposition);

  SECTION("density") {
    CHECK(medium.getMassDensity({gCS, {0_m, 0_m, 3_m}}) /
              medium.getMassDensity({gCS, {0_m, 3_m, 0_m}}) ==
          Approx(1));
  }

  SECTION("vertical") {
    Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
    FlatExponential<IMediumModel> const flat(gOrigin, axis, rho0, lambda,
                                             protonComposition);
    Line const line({gCS, {0_m, 0_m, 1_m}},
                    Vector<SpeedType::dimension_type>(
                        gCS, {0_m / second, 0_m / second, 5_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);

    CHECK(medium.getMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude() ==
          flat.getMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude());
    CHECK(medium.getIntegratedGrammage(trajectory).magnitude() ==
          flat.getIntegratedGrammage(trajectory).magnitude());
    CHECK(medium.getArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude() ==
          flat.getArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude());
  }
}

MassDensityType constexpr rho0 = 1_kg / 1_m / 1_m / 1_m;

struct ExponentialTest {
  auto operator()(Point const& p) const {
    return exp(p.getCoordinates()[0] / 1_m) * rho0;
  }

  template <int N>
  auto getDerivative(Point const& p, DirectionVector const& v) const {
    return v.getComponents()[0] * (*this)(p) / static_pow<N>(1_m);
  }

  auto getFirstDerivative(Point const& p, DirectionVector const& v) const {
    return getDerivative<1>(p, v);
  }

  auto getSecondDerivative(Point const& p, DirectionVector const& v) const {
    return getDerivative<2>(p, v);
  }
};

TEST_CASE("InhomogeneousMedium") {

  logging::set_level(logging::level::info);

  Vector direction(gCS, QuantityVector<dimensionless_d>(1, 0, 0));

  SpeedType const speed = 20_m / second;
  Line line(gOrigin, Vector<SpeedType::dimension_type>(
                         gCS, {speed, SpeedType::zero(), SpeedType::zero()}));

  // the tested LinearApproximationIntegrator really does a single step only. It is very
  // poor for exponentials with a bit larger step-width.
  TimeType const tEnd = 0.001_s;
  setup::Trajectory const trajectory =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  ExponentialTest const expTest;
  DensityFunction<ExponentialTest, LinearApproximationIntegrator> const rho(expTest);

  SECTION("DensityFunction") {
    CHECK(expTest.getDerivative<1>(gOrigin, direction) / (1_kg / 1_m / 1_m / 1_m / 1_m) ==
          Approx(1));
    CHECK(rho.evaluateAt(gOrigin) == expTest(gOrigin));
  }

  auto const exactGrammage = [](auto l) { return 1_m * rho0 * (exp(l / 1_m) - 1); };
  auto const exactLength = [](auto X) { return 1_m * log(1 + X / (rho0 * 1_m)); };

  LengthType constexpr length = tEnd * speed;

  NuclearComposition const composition{{Code::Proton}, {1.f}};
  InhomogeneousMedium<IMediumModel, decltype(rho)> const inhMedium(composition, rho);

  CORSIKA_LOG_INFO("test={} l={} {} {}", rho.getIntegrateGrammage(trajectory), length,
                   exactGrammage(length), 1_m * rho0 * (exp(length / 1_m) - 1));

  SECTION("Integration") {
    CORSIKA_LOG_INFO("test={} {} {}", rho.getIntegrateGrammage(trajectory),
                     exactGrammage(length),
                     rho.getIntegrateGrammage(trajectory) / exactGrammage(length));
    CHECK(rho.getIntegrateGrammage(trajectory) / exactGrammage(length) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.getArclengthFromGrammage(trajectory, exactGrammage(length)) /
              exactLength(exactGrammage(length)) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.getMaximumLength(trajectory, 1e-2) >
          length); // todo: write reasonable test when implementation is working

    CHECK(rho.getIntegrateGrammage(trajectory) ==
          inhMedium.getIntegratedGrammage(trajectory));
    CHECK(rho.getArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)) ==
          inhMedium.getArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)));
    CHECK(inhMedium.getNuclearComposition() == composition);
    CHECK(inhMedium.getMassDensity({gCS, {0_m, 0_m, 0_m}}) == 1_kg / static_pow<3>(1_m));
  }
}

TEST_CASE("LayeredSphericalAtmosphereBuilder") {

  logging::set_level(logging::level::info);

  LayeredSphericalAtmosphereBuilder builder =
      make_layered_spherical_atmosphere_builder<>::create(gOrigin,
                                                          constants::EarthRadius::Mean);

  builder.setNuclearComposition({{{Code::Nitrogen, Code::Oxygen}}, {{.6, .4}}});

  builder.addLinearLayer(1_km, 10_km);
  builder.addLinearLayer(2_km, 20_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 30_km);

  CHECK_THROWS(builder.addLinearLayer(0.5_km, 5_km));

  CHECK(builder.getSize() == 3);

  auto const builtEnv = builder.assemble();
  auto const& univ = builtEnv.getUniverse();

  CHECK(builder.getSize() == 0);

  auto const R = builder.getEarthRadius();

  CHECK(univ->getChildNodes().size() == 1);

  CHECK(univ->getContainingNode(Point(gCS, 0_m, 0_m, R + 35_km)) == univ.get());
  CHECK(dynamic_cast<Sphere const&>(
            univ->getContainingNode(Point(gCS, 0_m, 0_m, R + 8_km))->getVolume())
            .getRadius() == R + 10_km);
  CHECK(dynamic_cast<Sphere const&>(
            univ->getContainingNode(Point(gCS, 0_m, 0_m, R + 12_km))->getVolume())
            .getRadius() == R + 20_km);
  CHECK(dynamic_cast<Sphere const&>(
            univ->getContainingNode(Point(gCS, 0_m, 0_m, R + 24_km))->getVolume())
            .getRadius() == R + 30_km);
}

TEST_CASE("LayeredSphericalAtmosphereBuilder w/ magnetic field") {

  logging::set_level(logging::level::info);

  // setup our interface types
  using ModelInterface = IMagneticFieldModel<IMediumModel>;

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // create magnetic field vectors
  Vector B0(gCS, 0_T, 0_T, 1_T);

  LayeredSphericalAtmosphereBuilder builder = make_layered_spherical_atmosphere_builder<
      ModelInterface, UniformMagneticField>::create(gOrigin, constants::EarthRadius::Mean,
                                                    B0);

  builder.setNuclearComposition({{{Code::Nitrogen, Code::Oxygen}}, {{.6, .4}}});
  builder.addLinearLayer(1_km, 10_km);
  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 20_km);

  CHECK(builder.getSize() == 2);

  auto const builtEnv = builder.assemble();
  auto const& univ = builtEnv.getUniverse();

  CHECK(builder.getSize() == 0);
  CHECK(univ->getChildNodes().size() == 1);
  auto const R = builder.getEarthRadius();

  // check magnetic field at several locations
  const Point pTest(gCS, -10_m, 4_m, R + 35_m);
  CHECK(B0.getComponents(gCS) == univ->getContainingNode(pTest)
                                     ->getModelProperties()
                                     .getMagneticField(pTest)
                                     .getComponents(gCS));
  const Point pTest2(gCS, 10_m, -4_m, R + 15_km);
  CHECK(B0.getComponents(gCS) == univ->getContainingNode(pTest2)
                                     ->getModelProperties()
                                     .getMagneticField(pTest2)
                                     .getComponents(gCS));
}

TEST_CASE("media", "LayeredSphericalAtmosphereBuilder USStd") {
  // setup environment, geometry
  Point const center{gCS, 0_m, 0_m, 0_m};

  // setup our interface types
  auto builder = make_layered_spherical_atmosphere_builder<>::create(
      center, constants::EarthRadius::Mean);

  builder.setNuclearComposition(
      {{Code::Nitrogen, Code::Oxygen},
       {0.7847f, 1.f - 0.7847f}}); // values taken from AIRES manual, Ar removed for now

  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 4_km);
  builder.addExponentialLayer(1144.9069_g / (1_cm * 1_cm), 878153.55_cm, 10_km);
  builder.addExponentialLayer(1305.5948_g / (1_cm * 1_cm), 636143.04_cm, 40_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 100_km);
  builder.addLinearLayer(1e9_cm, 112.8_km);

  Environment<IMediumModel> env;
  builder.assemble(env);

  typedef typename Environment<IMediumModel>::BaseNodeType::VTN_type node_type;
  node_type const* universe = env.getUniverse().get();

  // far out ther is the universe
  CHECK(universe->getContainingNode(Point(gCS, {10000_km, 0_m, 0_m})) == universe);
  CHECK(universe->getContainingNode(Point(gCS, {0_m, 10000_km, 0_m})) == universe);

  // at 112.8km there is transition to atmosphere
  CHECK(universe->getContainingNode(
            Point(gCS, {constants::EarthRadius::Mean + 112.8_km + 1_cm, 0_m, 0_m})) ==
        universe);
  CHECK(universe->getContainingNode(
            Point(gCS, {0_m, constants::EarthRadius::Mean + 112.8_km + 1_cm, 0_m})) ==
        universe);

  // check layer transition at 112.8km

  node_type const* layer1_not_yet = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 112.8_km + 1_cm, 0_m, 0_m}));
  node_type const* layer1 = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 112.8_km - 1_cm, 0_m, 0_m}));
  node_type const* layer1_also = universe->getContainingNode(
      Point(gCS, {0_m, constants::EarthRadius::Mean + 112.8_km - 1_cm, 0_m}));

  CHECK(layer1_not_yet == universe);
  CHECK(layer1 != universe);
  CHECK(layer1 == layer1_also);

  // check layer transition at 100km

  node_type const* layer2_not_yet = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 100_km + 1_cm, 0_m, 0_m}));
  node_type const* layer2 = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 100_km - 1_cm, 0_m, 0_m}));
  node_type const* layer2_also = universe->getContainingNode(
      Point(gCS, {0_m, constants::EarthRadius::Mean + 100_km - 1_cm, 0_m}));

  CHECK(layer2_not_yet == layer1);
  CHECK(layer2 != layer1);
  CHECK(layer2 == layer2_also);

  // check layer transition at 40km

  node_type const* layer3_not_yet = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 40_km + 1_cm, 0_m, 0_m}));
  node_type const* layer3 = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 40_km - 1_cm, 0_m, 0_m}));
  node_type const* layer3_also = universe->getContainingNode(
      Point(gCS, {0_m, constants::EarthRadius::Mean + 40_km - 1_cm, 0_m}));

  CHECK(layer3_not_yet == layer2);
  CHECK(layer3 != layer2);
  CHECK(layer3 == layer3_also);

  // check layer transition at 10km

  node_type const* layer4_not_yet = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 10_km + 1_cm, 0_m, 0_m}));
  node_type const* layer4 = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 10_km - 1_cm, 0_m, 0_m}));
  node_type const* layer4_also = universe->getContainingNode(
      Point(gCS, {0_m, constants::EarthRadius::Mean + 10_km - 1_cm, 0_m}));

  CHECK(layer4_not_yet == layer3);
  CHECK(layer4 != layer3);
  CHECK(layer4 == layer4_also);

  // check layer transition at 4km

  node_type const* layer5_not_yet = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 4_km + 1_cm, 0_m, 0_m}));
  node_type const* layer5 = universe->getContainingNode(
      Point(gCS, {constants::EarthRadius::Mean + 4_km - 1_cm, 0_m, 0_m}));
  node_type const* layer5_also = universe->getContainingNode(
      Point(gCS, {0_m, constants::EarthRadius::Mean + 4_km - 1_cm, 0_m}));

  CHECK(layer5_not_yet == layer4);
  CHECK(layer5 != layer4);
  CHECK(layer5 == layer5_also);
}
