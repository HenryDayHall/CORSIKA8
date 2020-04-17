/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/InhomogeneousMedium.hpp>
#include <corsika/media/LayeredSphericalAtmosphereBuilder.hpp>
#include <corsika/media/LinearApproximationIntegrator.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/SlidingPlanarExponential.hpp>
#include <corsika/media/VolumeTreeNode.hpp>

#include <corsika/setup/SetupTrajectory.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::units::si;

CoordinateSystem const& gCS =
    RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

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
  auto const rho0 = 1_g / units::static_pow<3>(1_cm);
  FlatExponential<IMediumModel> const medium(gOrigin, axis, rho0, lambda,
                                             protonComposition);
  auto const tEnd = 5_s;

  SECTION("horizontal") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {20_cm / second, 0_m / second, 0_m / second}));
    setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);
    CHECK((medium.IntegratedGrammage(trajectory, 2_m) / (rho0 * 2_m)) == Approx(1));
    CHECK((medium.ArclengthFromGrammage(trajectory, rho0 * 5_m) / 5_m) == Approx(1));
  }

  SECTION("vertical") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, 5_m / second}));
    setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);
    LengthType const length = 2 * lambda;
    GrammageType const exact = rho0 * lambda * (exp(length / lambda) - 1);

    CHECK((medium.IntegratedGrammage(trajectory, length) / exact) == Approx(1));
    CHECK((medium.ArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }

  SECTION("escape grammage") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, -5_m / second}));
    setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

    GrammageType const escapeGrammage = rho0 * lambda;

    CHECK(trajectory.GetDirection(0).dot(axis).magnitude() < 0);
    CHECK(medium.ArclengthFromGrammage(trajectory, 1.2 * escapeGrammage) ==
          std::numeric_limits<typename GrammageType::value_type>::infinity() * 1_m);
  }

  SECTION("inclined") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 5_m / second, 5_m / second}));
    setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);
    double const cosTheta = M_SQRT1_2;
    LengthType const length = 2 * lambda;
    GrammageType const exact =
        rho0 * lambda * (exp(cosTheta * length / lambda) - 1) / cosTheta;
    CHECK((medium.IntegratedGrammage(trajectory, length) / exact) == Approx(1));
    CHECK((medium.ArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }
}

TEST_CASE("SlidingPlanarExponential") {
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  LengthType const lambda = 3_m;
  auto const rho0 = 1_g / units::static_pow<3>(1_cm);
  auto const tEnd = 5_s;

  SlidingPlanarExponential<IMediumModel> const medium(gOrigin, rho0, lambda,
                                                      protonComposition);

  SECTION("density") {
    CHECK(medium.GetMassDensity({gCS, {0_m, 0_m, 3_m}}) /
              medium.GetMassDensity({gCS, {0_m, 3_m, 0_m}}) ==
          Approx(1));
  }

  SECTION("vertical") {
    Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));
    FlatExponential<IMediumModel> const flat(gOrigin, axis, rho0, lambda,
                                             protonComposition);
    Line const line({gCS, {0_m, 0_m, 1_m}},
                    Vector<SpeedType::dimension_type>(
                        gCS, {0_m / second, 0_m / second, 5_m / second}));
    setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

    CHECK(medium.GetMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude() ==
          flat.GetMassDensity({gCS, {0_mm, 0_m, 3_m}}).magnitude());
    CHECK(medium.IntegratedGrammage(trajectory, 2_m).magnitude() ==
          flat.IntegratedGrammage(trajectory, 2_m).magnitude());
    CHECK(medium.ArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude() ==
          flat.ArclengthFromGrammage(trajectory, rho0 * 5_m).magnitude());
  }
}

auto constexpr rho0 = 1_kg / 1_m / 1_m / 1_m;

struct Exponential {
  auto operator()(corsika::Point const& p) const {
    return exp(p.GetCoordinates()[0] / 1_m) * rho0;
  }

  template <int N>
  auto Derivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return v.GetComponents()[0] * (*this)(p) / corsika::units::static_pow<N>(1_m);
  }

  auto FirstDerivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return Derivative<1>(p, v);
  }

  auto SecondDerivative(Point const& p, Vector<dimensionless_d> const& v) const {
    return Derivative<2>(p, v);
  }
};

TEST_CASE("InhomogeneousMedium") {
  Vector direction(gCS, QuantityVector<dimensionless_d>(1, 0, 0));

  Line line(gOrigin, Vector<SpeedType::dimension_type>(
                         gCS, {20_m / second, 0_m / second, 0_m / second}));

  auto const tEnd = 5_s;
  setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

  Exponential const e;
  DensityFunction<decltype(e), LinearApproximationIntegrator> const rho(e);

  SECTION("DensityFunction") {
    CHECK(e.Derivative<1>(gOrigin, direction) / (1_kg / 1_m / 1_m / 1_m / 1_m) ==
          Approx(1));
    CHECK(rho.EvaluateAt(gOrigin) == e(gOrigin));
  }

  auto const exactGrammage = [](auto l) { return 1_m * rho0 * (exp(l / 1_m) - 1); };
  auto const exactLength = [](auto X) { return 1_m * log(1 + X / (rho0 * 1_m)); };

  auto constexpr l = 15_cm;

  NuclearComposition const composition{{Code::Proton}, {1.f}};
  InhomogeneousMedium<IMediumModel, decltype(rho)> const inhMedium(composition, rho);

  SECTION("Integration") {
    CHECK(rho.IntegrateGrammage(trajectory, l) / exactGrammage(l) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.ArclengthFromGrammage(trajectory, exactGrammage(l)) /
              exactLength(exactGrammage(l)) ==
          Approx(1).epsilon(1e-2));
    CHECK(rho.MaximumLength(trajectory, 1e-2) >
          l); // todo: write reasonable test when implementation is working

    CHECK(rho.IntegrateGrammage(trajectory, l) ==
          inhMedium.IntegratedGrammage(trajectory, l));
    CHECK(rho.ArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)) ==
          inhMedium.ArclengthFromGrammage(trajectory, 20_g / (1_cm * 1_cm)));
  }
}

TEST_CASE("LayeredSphericalAtmosphereBuilder") {
  LayeredSphericalAtmosphereBuilder builder(gOrigin);
  builder.setNuclearComposition({{{Code::Nitrogen, Code::Oxygen}}, {{.6, .4}}});

  builder.addLinearLayer(1_km, 10_km);
  builder.addLinearLayer(2_km, 20_km);
  builder.addExponentialLayer(540.1778_g / (1_cm * 1_cm), 772170.16_cm, 30_km);

  CHECK(builder.size() == 3);

  auto const builtEnv = builder.assemble();
  auto const& univ = builtEnv.GetUniverse();

  CHECK(builder.size() == 0);

  auto const R = builder.getEarthRadius();

  CHECK(univ->GetChildNodes().size() == 1);

  CHECK(univ->GetContainingNode(Point(gCS, 0_m, 0_m, R + 35_km)) == univ.get());
  CHECK(dynamic_cast<Sphere const&>(
            univ->GetContainingNode(Point(gCS, 0_m, 0_m, R + 8_km))->GetVolume())
            .GetRadius() == R + 10_km);
  CHECK(dynamic_cast<Sphere const&>(
            univ->GetContainingNode(Point(gCS, 0_m, 0_m, R + 12_km))->GetVolume())
            .GetRadius() == R + 20_km);
  CHECK(dynamic_cast<Sphere const&>(
            univ->GetContainingNode(Point(gCS, 0_m, 0_m, R + 24_km))->GetVolume())
            .GetRadius() == R + 30_km);
}

TEST_CASE("UniformMagneticField w/ Homogeneous Medium") {

  // setup our interface types
  using IModelInterface = IMagneticFieldModel<IMediumModel>;
  using AtmModel = UniformMagneticField<HomogeneousMedium<IModelInterface>>;

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // create a magnetic field vector
  Vector B0(gCS, 0_T, 0_T, 0_T);

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // create our atmospheric model
  AtmModel medium(B0, density, protonComposition);

  // and test at several locations
  CHECK(B0.GetComponents(gCS) ==
        medium.GetMagneticField(Point(gCS, -10_m, 4_m, 35_km)).GetComponents(gCS));
  CHECK(
      B0.GetComponents(gCS) ==
      medium.GetMagneticField(Point(gCS, 1000_km, -1000_km, 1000_km)).GetComponents(gCS));
  CHECK(B0.GetComponents(gCS) ==
        medium.GetMagneticField(Point(gCS, 0_m, 0_m, 0_m)).GetComponents(gCS));

  // create a new magnetic field vector
  Vector B1(gCS, 23_T, 57_T, -4_T);

  // and update this atmospheric model
  medium.SetMagneticField(B1);

  // and test at several locations
  CHECK(B1.GetComponents(gCS) ==
        medium.GetMagneticField(Point(gCS, -10_m, 4_m, 35_km)).GetComponents(gCS));
  CHECK(
      B1.GetComponents(gCS) ==
      medium.GetMagneticField(Point(gCS, 1000_km, -1000_km, 1000_km)).GetComponents(gCS));
  CHECK(B1.GetComponents(gCS) ==
        medium.GetMagneticField(Point(gCS, 0_m, 0_m, 0_m)).GetComponents(gCS));

  // check the density and nuclear composition
  CHECK(density == medium.GetMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.GetNuclearComposition();

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("LayeredSphericalAtmosphereBuilder w/ magnetic field") {

  // setup our interface types
  using ModelInterface = IMagneticFieldModel<IMediumModel>;

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // create magnetic field vectors
  Vector B0(gCS, 0_T, 0_T, 1_T);

  LayeredSphericalAtmosphereBuilder builder =
      environment::make_layered_spherical_atmosphere_builder<
          ModelInterface,
          UniformMagneticField>::create(gOrigin, units::constants::EarthRadius::Mean, B0);

  builder.setNuclearComposition(
      {{{particles::Code::Nitrogen, particles::Code::Oxygen}}, {{.6, .4}}});
  builder.addLinearLayer(1_km, 10_km);
  builder.addExponentialLayer(1222.6562_g / (1_cm * 1_cm), 994186.38_cm, 20_km);

  CHECK(builder.size() == 2);

  auto const builtEnv = builder.assemble();
  auto const& univ = builtEnv.GetUniverse();

  CHECK(builder.size() == 0);
  CHECK(univ->GetChildNodes().size() == 1);
  auto const R = builder.getEarthRadius();

  // check magnetic field at several locations
  const Point pTest(gCS, -10_m, 4_m, R + 35_m);
  CHECK(B0.GetComponents(gCS) == univ->GetContainingNode(pTest)
                                     ->GetModelProperties()
                                     .GetMagneticField(pTest)
                                     .GetComponents(gCS));
  const Point pTest2(gCS, 10_m, -4_m, R + 15_km);
  CHECK(B0.GetComponents(gCS) == univ->GetContainingNode(pTest2)
                                     ->GetModelProperties()
                                     .GetMagneticField(pTest2)
                                     .GetComponents(gCS));
}

TEST_CASE("UniformRefractiveIndex w/ Homogeneous") {

  // setup our interface types
  using IModelInterface = IRefractiveIndexModel<IMediumModel>;
  using AtmModel = UniformRefractiveIndex<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // the refrative index that we use
  const double n{1.000327};

  // create the atmospheric model
  AtmModel medium(n, density, protonComposition);

  // and require that it is constant
  CHECK(n == medium.GetRefractiveIndex(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(n == medium.GetRefractiveIndex(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(n == medium.GetRefractiveIndex(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(n == medium.GetRefractiveIndex(Point(gCS, 100_km, 400_km, 350_km)));

  // a new refractive index
  const double n2{2.3472123};

  // update the refractive index of this atmospheric model
  medium.SetRefractiveIndex(n2);

  // check that the returned refractive index is correct
  CHECK(n2 == medium.GetRefractiveIndex(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(n2 == medium.GetRefractiveIndex(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(n2 == medium.GetRefractiveIndex(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(n2 == medium.GetRefractiveIndex(Point(gCS, 100_km, 400_km, 350_km)));

  // define our axis vector
  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));

  // check the density and nuclear composition
  CHECK(density == medium.GetMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.GetNuclearComposition();

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("MediumProperties") {

  // test access of medium properties via enum and class types

  const Medium type = Medium::AirDry1Atm;
  const MediumData& air = mediumData(type);
  CHECK(air.Ieff() == 85.7);
  CHECK(air.Cbar() == 10.5961);
  CHECK(air.x0() == 1.7418);
  CHECK(air.x1() == 4.2759);
  CHECK(air.aa() == 0.10914);
  CHECK(air.sk() == 3.3994);
  CHECK(air.dlt0() == 0.0);
}

TEST_CASE("MediumPropertyModel w/ Homogeneous") {

  // setup our interface types
  using IModelInterface = IMediumPropertyModel<IMediumModel>;
  using AtmModel = MediumPropertyModel<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // the refrative index that we use
  const Medium type = Medium::AirDry1Atm;

  // create the atmospheric model
  AtmModel medium(type, density, protonComposition);

  // and require that it is constant
  CHECK(type == medium.medium(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type == medium.medium(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type == medium.medium(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type == medium.medium(Point(gCS, 100_km, 400_km, 350_km)));

  // a new refractive index
  const Medium type2 = Medium::StandardRock;

  // update the refractive index of this atmospheric model
  medium.set_medium(type2);

  // check that the returned refractive index is correct
  CHECK(type2 == medium.medium(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type2 == medium.medium(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type2 == medium.medium(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type2 == medium.medium(Point(gCS, 100_km, 400_km, 350_km)));

  // define our axis vector
  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));

  // check the density and nuclear composition
  CHECK(density == medium.GetMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.GetNuclearComposition();

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  setup::Trajectory const trajectory = setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}
