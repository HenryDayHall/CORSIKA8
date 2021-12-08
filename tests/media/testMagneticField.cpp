/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/UniformMagneticField.hpp>
#include <corsika/media/IMagneticFieldModel.hpp>
#include <corsika/media/VolumeTreeNode.hpp>
#include <corsika/media/WMM.hpp>

#include <SetupTestTrajectory.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("UniformMagneticField w/ Homogeneous Medium") {

  logging::set_level(logging::level::info);

  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();
  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  // setup our interface types
  using IModelInterface = IMagneticFieldModel<IMediumModel>;
  using AtmModel = UniformMagneticField<HomogeneousMedium<IModelInterface>>;

  // the composition we use for the homogenous medium
  NuclearComposition const protonComposition({Code::Proton}, {1.});

  // create a magnetic field vector
  Vector B0(gCS, 0_T, 0_T, 0_T);

  // the constant density
  const auto density{19.2_g / cube(1_cm)};

  // create our atmospheric model
  AtmModel medium(B0, density, protonComposition);

  // and test at several locations
  CHECK(B0.getComponents(gCS) ==
        medium.getMagneticField(Point(gCS, -10_m, 4_m, 35_km)).getComponents(gCS));
  CHECK(
      B0.getComponents(gCS) ==
      medium.getMagneticField(Point(gCS, 1000_km, -1000_km, 1000_km)).getComponents(gCS));
  CHECK(B0.getComponents(gCS) ==
        medium.getMagneticField(Point(gCS, 0_m, 0_m, 0_m)).getComponents(gCS));

  // create a new magnetic field vector
  Vector B1(gCS, 23_T, 57_T, -4_T);

  // and update this atmospheric model
  medium.setMagneticField(B1);

  // and test at several locations
  CHECK(B1.getComponents(gCS) ==
        medium.getMagneticField(Point(gCS, -10_m, 4_m, 35_km)).getComponents(gCS));
  CHECK(
      B1.getComponents(gCS) ==
      medium.getMagneticField(Point(gCS, 1000_km, -1000_km, 1000_km)).getComponents(gCS));
  CHECK(B1.getComponents(gCS) ==
        medium.getMagneticField(Point(gCS, 0_m, 0_m, 0_m)).getComponents(gCS));

  // check the density and nuclear composition
  CHECK(density == medium.getMassDensity(Point(gCS, 0_m, 0_m, 0_m)));
  medium.getNuclearComposition();

  // create a line of length 1 m
  Line const line(gOrigin, Vector<SpeedType::dimension_type>(
                               gCS, {1_m / second, 0_m / second, 0_m / second}));

  // the end time of our line
  auto const tEnd = 1_s;

  // and the associated trajectory
  setup::Trajectory const track =
      setup::testing::make_track<setup::Trajectory>(line, tEnd);

  // create earth magnetic field vector
  MagneticFieldVector Earth_B_1 = get_wmm(gCS, 2022.5, 100_km, -80, -120);
  MagneticFieldVector Earth_B_2 = get_wmm(gCS, 2022.5, 0_km, 0, 120);
  MagneticFieldVector Earth_B_3 = get_wmm(gCS, 2020, 100_km, 80, 0);
  CHECK(Earth_B_1.getX(gCS) / 1_nT == Approx(5815).margin(0.03));
  CHECK(Earth_B_1.getY(gCS) / 1_nT == Approx(14803).margin(0.03));
  CHECK(Earth_B_1.getZ(gCS) / 1_nT == Approx(49755.3).margin(0.03));
  CHECK(Earth_B_2.getX(gCS) / 1_nT == Approx(39684.7).margin(0.03));
  CHECK(Earth_B_2.getY(gCS) / 1_nT == Approx(-42.2).margin(0.03));
  CHECK(Earth_B_2.getZ(gCS) / 1_nT == Approx(10809.5).margin(0.03));
  CHECK(Earth_B_3.getX(gCS) / 1_nT == Approx(6261.8).margin(0.03));
  CHECK(Earth_B_3.getY(gCS) / 1_nT == Approx(-185.5).margin(0.03));
  CHECK(Earth_B_3.getZ(gCS) / 1_nT == Approx(-52429.1).margin(0.03));
}
