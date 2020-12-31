/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/InhomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/UniformRefractiveIndex.hpp>

#include <SetupTestTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("UniformRefractiveIndex w/ Homogeneous") {

  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();

  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

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

  // create a line of length 1 m
  Line const line(gOrigin,
                  VelocityVector(gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  setup::Trajectory const track =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.getIntegratedGrammage(track, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.getArclengthFromGrammage(track, density * 5_m) / 5_m) == Approx(1));
}
