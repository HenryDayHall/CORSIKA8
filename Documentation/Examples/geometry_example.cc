/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

#include <cstdlib>
#include <iostream>
#include <typeinfo>

using namespace corsika;
using namespace corsika::geometry;
using namespace corsika::units::si;

int main() {
  // define the root coordinate system
  geometry::CoordinateSystem& root =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  // another CS defined by a translation relative to the root CS
  CoordinateSystem cs2 = root.translate({0_m, 0_m, 1_m});

  // rotations are possible, too; parameters are axis vector and angle
  CoordinateSystem cs3 =
      root.rotate(QuantityVector<length_d>{1_m, 0_m, 0_m}, 90 * degree_angle);

  // now let's define some geometrical objects:
  Point const p1(root, {0_m, 0_m, 0_m}); // the origin of the root CS
  Point const p2(cs2, {0_m, 0_m, 0_m});  // the origin of cs2

  Vector<length_d> const diff =
      p2 -
      p1; // the distance between the points, basically the translation vector given above
  auto const norm = diff.squaredNorm(); // squared length with the right dimension

  // print the components of the vector as given in the different CS
  std::cout << "p2-p1 components in root: " << diff.GetComponents(root) << std::endl;
  std::cout << "p2-p1 components in cs2: " << diff.GetComponents(cs2)
            << std::endl; // by definition invariant under translations
  std::cout << "p2-p1 components in cs3: " << diff.GetComponents(cs3)
            << std::endl; // but not under rotations
  std::cout << "p2-p1 norm^2: " << norm << std::endl;
  assert(norm == 1 * meter * meter);

  Sphere s(p1, 10_m); // define a sphere around a point with a radius
  std::cout << "p1 inside s:  " << s.Contains(p2) << std::endl;
  assert(s.Contains(p2) == 1);

  Sphere s2(p1, 3_um); // another sphere
  std::cout << "p1 inside s2: " << s2.Contains(p2) << std::endl;
  assert(s2.Contains(p2) == 0);

  // let's try parallel projections:
  auto const v1 = Vector<length_d>(root, {1_m, 1_m, 0_m});
  auto const v2 = Vector<length_d>(root, {1_m, 0_m, 0_m});

  auto const v3 = v1.parallelProjectionOnto(v2);

  // cross product
  auto const cross =
      v1.cross(v2).normalized(); // normalized() returns dimensionless, normalized vectors

  // if a CS is not given as parameter for getComponents(), the components
  // in the "home" CS are returned
  std::cout << "v1: " << v1.GetComponents() << std::endl;
  std::cout << "v2: " << v2.GetComponents() << std::endl;
  std::cout << "parallel projection of v1 onto v2: " << v3.GetComponents() << std::endl;
  std::cout << "normalized cross product of v1 x v2" << cross.GetComponents()
            << std::endl;

  return EXIT_SUCCESS;
}
