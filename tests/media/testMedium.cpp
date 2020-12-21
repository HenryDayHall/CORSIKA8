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
#include <corsika/media/MediumPropertyModel.hpp>
#include <corsika/media/MediumProperties.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("MediumProperties") {

  // test access of medium properties via enum and class types

  const Medium type = Medium::AirDry1Atm;
  const MediumData& air = mediumData(type);
  CHECK(air.getIeff() == 85.7);
  CHECK(air.getCbar() == 10.5961);
  CHECK(air.getX0() == 1.7418);
  CHECK(air.getX1() == 4.2759);
  CHECK(air.getAA() == 0.10914);
  CHECK(air.getSK() == 3.3994);
  CHECK(air.getDlt0() == 0.0);
}


TEST_CASE("MediumPropertyModel w/ Homogeneous") {

  CoordinateSystemPtr gCS = get_root_CoordinateSystem();

  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  // setup our interface types
  using IModelInterface = IMediumPropertyModel<IMediumModel>;
  using AtmModel = MediumPropertyModel<HomogeneousMedium<IModelInterface>>;

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition(std::vector<Code>{Code::Proton},
                                             std::vector<float>{1.f});

  // the refrative index that we use
  const Medium type = corsika::Medium::AirDry1Atm;

  // create the atmospheric model
  AtmModel medium(type, density, protonComposition);

  // and require that it is constant
  CHECK(type == medium.getMedium(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type == medium.getMedium(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type == medium.getMedium(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type == medium.getMedium(Point(gCS, 100_km, 400_km, 350_km)));

  // a new refractive index
  const Medium type2 = corsika::Medium::StandardRock;

  // update the refractive index of this atmospheric model
  medium.setMedium(type2);

  // check that the returned refractive index is correct
  CHECK(type2 == medium.getMedium(Point(gCS, -10_m, 4_m, 35_km)));
  CHECK(type2 == medium.getMedium(Point(gCS, +210_m, 0_m, 7_km)));
  CHECK(type2 == medium.getMedium(Point(gCS, 0_m, 0_m, 0_km)));
  CHECK(type2 == medium.getMedium(Point(gCS, 100_km, 400_km, 350_km)));

  // define our axis vector
  Vector const axis(gCS, QuantityVector<dimensionless_d>(0, 0, 1));

  // check the density and nuclear composition
  CHECK(density == medium.getMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.getNuclearComposition();

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  Trajectory<Line> const trajectory(line, tEnd);

  // and check the integrated grammage
  CHECK((medium.getIntegratedGrammage(trajectory, 3_m) / (density * 3_m)) == Approx(1));
  CHECK((medium.getArclengthFromGrammage(trajectory, density * 5_m) / 5_m) == Approx(1));
}
