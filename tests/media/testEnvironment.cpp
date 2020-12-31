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

#include <catch2/catch.hpp>

using namespace corsika;

CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

Point const gOrigin(gCS, {0_m, 0_m, 0_m});

TEST_CASE("HomogeneousMedium") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);
}

TEST_CASE("FlatExponential") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / static_pow<3>(1_cm);
  FlatExponential<IMediumModel> const medium(gOrigin, axis, rho0, lambda,
                                             protonComposition);
  auto const tEnd = 5_s;

  SECTION("horizontal") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {20_cm / second, 0_m / second, 0_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);

    CHECK((medium.getIntegratedGrammage(trajectory, 2_m) / (rho0 * 2_m)) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, rho0 * 5_m) / 5_m) == Approx(1));
  }

  SECTION("vertical") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, 5_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    LengthType const length = 2 * lambda;
    GrammageType const exact = rho0 * lambda * (exp(length / lambda) - 1);

    CHECK((medium.getIntegratedGrammage(trajectory, length) / exact) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }

  SECTION("escape grammage") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, -5_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    GrammageType const escapeGrammage = rho0 * lambda;

    CHECK(trajectory.getDirection(0).dot(axis).magnitude() < 0);
    CHECK(medium.getArclengthFromGrammage(trajectory, 1.2 * escapeGrammage) ==
          std::numeric_limits<typename GrammageType::value_type>::infinity() * 1_m);
  }

  SECTION("inclined") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 5_m / second, 5_m / second}));
    setup::Trajectory const trajectory =
        setup::testing::make_track<setup::Trajectory>(line, tEnd);
    double const cosTheta = M_SQRT1_2;
    LengthType const length = 2 * lambda;
    GrammageType const exact =
        rho0 * lambda * (exp(cosTheta * length / lambda) - 1) / cosTheta;
    CHECK((medium.getIntegratedGrammage(trajectory, length) / exact) == Approx(1));
    CHECK((medium.getArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }
}

TEST_CASE("SlidingPlanarExponential") {
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
    CHECK(medium.getIntegratedGrammage(trajectory, 2_m).magnitude() ==
          flat.getIntegratedGrammage(trajectory, 2_m).magnitude());
    CHECK(medium.getArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude() ==
          flat.getArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude());
  }
}

auto constexpr rho0 = 1_kg / 1_m / 1_m / 1_m;

struct Exponential {
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
  Vector direction(gCS, QuantityVector<dimensionless_d>(1, 0, 0));

  Line line(gOrigin, Vector<SpeedType::dimension_type>(
                         gCS, {20_m / second, 0_m / second, 0_m / second}));

  auto const tEnd = 5_s;
  setup::Trajectory const trajectory =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  Exponential const e;
  DensityFunction<decltype(e), LinearApproximationIntegrator> const rho(e);

  SECTION("DensityFunction") {
    CHECK(e.getDerivative<1>(gOrigin, direction) / (1_kg / 1_m / 1_m / 1_m / 1_m) ==
          Approx(1));
    CHECK(rho.evaluateAt(gOrigin) == e(gOrigin));
  }

  auto const exactGrammage = [](auto l) { return 1_m * rho0 * (exp(l / 1_m) - 1); };
  auto const exactLength = [](auto X) { return 1_m * log(1 + X / (rho0 * 1_m)); };

  auto constexpr l = 15_cm;

  NuclearComposition const composition{{Code::Proton}, {1.f}};
  InhomogeneousMedium<IMediumModel, decltype(rho)> const inhMedium(composition, rho);

  SECTION("Integration") {
    CHECK(rho.getIntegrateGrammage(trajectory, l) / exactGrammage(l) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.getArclengthFromGrammage(trajectory, exactGrammage(l)) /
              exactLength(exactGrammage(l)) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.getMaximumLength(trajectory, 1e-2) >
          l); // todo: write reasonable test when implementation is working

    CHECK(rho.getIntegrateGrammage(trajectory, l) ==
          inhMedium.getIntegratedGrammage(trajectory, l));
    CHECK(rho.getArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)) ==
          inhMedium.getArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)));
  }
}

TEST_CASE("LayeredSphericalAtmosphereBuilder") {

  LayeredSphericalAtmosphereBuilder builder =
      make_layered_spherical_atmosphere_builder<>::create(gOrigin,
                                                          constants::EarthRadius::Mean);

  builder.setNuclearComposition({{{Code::Nitrogen, Code::Oxygen}}, {{.6, .4}}});

  builder.addLinearLayer(1_km, 10_km);
  builder.addLinearLayer(2_km, 20_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 30_km);

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
