/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/DensityFunction.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMagneticFieldModel.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/IMediumTypeModel.h>
#include <corsika/environment/IRefractiveIndexModel.h>
#include <corsika/environment/InhomogeneousMedium.h>
#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
#include <corsika/environment/LinearApproximationIntegrator.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/SlidingPlanarExponential.h>
#include <corsika/environment/UniformMagneticField.h>
#include <corsika/environment/UniformMediumType.h>
#include <corsika/environment/UniformRefractiveIndex.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

using namespace corsika::geometry;
using namespace corsika::environment;
using namespace corsika::particles;
using namespace corsika::units::si;
using namespace corsika;

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
    Trajectory<Line> const trajectory(line, tEnd);

    CHECK((medium.IntegratedGrammage(trajectory, 2_m) / (rho0 * 2_m)) == Approx(1));
    CHECK((medium.ArclengthFromGrammage(trajectory, rho0 * 5_m) / 5_m) == Approx(1));
  }

  SECTION("vertical") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, 5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);
    LengthType const length = 2 * lambda;
    GrammageType const exact = rho0 * lambda * (exp(length / lambda) - 1);

    CHECK((medium.IntegratedGrammage(trajectory, length) / exact) == Approx(1));
    CHECK((medium.ArclengthFromGrammage(trajectory, exact) / length) == Approx(1));
  }

  SECTION("escape grammage") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 0_m / second, -5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);

    GrammageType const escapeGrammage = rho0 * lambda;

    CHECK(trajectory.NormalizedDirection().dot(axis).magnitude() < 0);
    CHECK(medium.ArclengthFromGrammage(trajectory, 1.2 * escapeGrammage) ==
          std::numeric_limits<typename GrammageType::value_type>::infinity() * 1_m);
  }

  SECTION("inclined") {
    Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                                 gCS, {0_m / second, 5_m / second, 5_m / second}));
    Trajectory<Line> const trajectory(line, tEnd);
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
    Trajectory<Line> const trajectory(line, tEnd);

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
  auto operator()(corsika::geometry::Point const& p) const {
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
  Trajectory<Line> const trajectory(line, tEnd);

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
  builder.setNuclearComposition(
      {{{particles::Code::Nitrogen, particles::Code::Oxygen}}, {{.6, .4}}});

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
  Trajectory<Line> const trajectory(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("LayeredSphericalAtmosphereBuilder w/ magnetic field") {

  // setup our interface types
  using IModelInterface = IMagneticFieldModel<IMediumModel>;

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // create magnetic field vectors
  Vector B0(gCS, 0_T, 0_T, 1_T);
  Vector B1(gCS, 1_T, 1_T, 0_T);

  // LayeredSphericalAtmosphereBuilder<IMediumModel> builder(gOrigin);
  LayeredSphericalAtmosphereBuilder<IModelInterface> builder(gOrigin);
  builder.setNuclearComposition(
      {{{particles::Code::Nitrogen, particles::Code::Oxygen}}, {{.6, .4}}});

  builder.addLinearLayer<UniformMagneticField>(1_km, 10_km, B0);
  builder.addExponentialLayer<UniformMagneticField>(1222.6562_g / (1_cm * 1_cm),
                                                    994186.38_cm, 20_km, B1);

  CHECK(builder.size() == 2);

  auto const builtEnv = builder.assemble();
  auto const& univ = builtEnv.GetUniverse();

  CHECK(builder.size() == 0);
  CHECK(univ->GetChildNodes().size() == 1);
  auto const R = builder.getEarthRadius();

  // check magnetic field at several locations
  const Point pTest(gCS, -10_m, 4_m, R+35_m);
  CHECK(B0.GetComponents(gCS) == univ->GetContainingNode(pTest)
                                     ->GetModelProperties()
                                     .GetMagneticField(pTest)
                                     .GetComponents(gCS));
  const Point pTest2(gCS, 10_m, -4_m, R+15_km);
  CHECK(B1.GetComponents(gCS) == univ->GetContainingNode(pTest2)
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
  Trajectory<Line> const trajectory(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}

TEST_CASE("UniformMediumType w/ Homogeneous") {

  // setup our interface types
  using IModelInterface = IMediumTypeModel<IMediumModel>;
  using AtmModel = UniformMediumType<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // the refrative index that we use
  const EMediumType type = EMediumType::eAir;

  // create the atmospheric model
  AtmModel medium(type, density, protonComposition);

  // and require that it is constant
  CHECK(type == medium.medium_type(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type == medium.medium_type(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type == medium.medium_type(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type == medium.medium_type(Point(gCS, 100_km, 400_km, 350_km)));

  // a new refractive index
  const EMediumType type2 = EMediumType::eRock;

  // update the refractive index of this atmospheric model
  medium.set_medium_type(type2);

  // check that the returned refractive index is correct
  CHECK(type2 == medium.medium_type(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type2 == medium.medium_type(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type2 == medium.medium_type(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type2 == medium.medium_type(Point(gCS, 100_km, 400_km, 350_km)));

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
  Trajectory<Line> const trajectory(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.IntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.ArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}
