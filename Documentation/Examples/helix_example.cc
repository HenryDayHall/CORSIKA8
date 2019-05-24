/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/Helix.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <array>
#include <cstdlib>
#include <iostream>

using namespace corsika;
using namespace corsika::geometry;
using namespace corsika::units::si;

int main() {
  geometry::CoordinateSystem& root =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  Point const r0(root, {0_m, 0_m, 0_m});
  auto const omegaC = 2 * M_PI * 1_Hz;
  Vector<speed_d> vPar(root, {0_m / second, 0_m / second, 10_cm / second});
  Vector<speed_d> vPerp(root, {1_m / second, 0_m / second, 0_m / second});

  Helix h(r0, omegaC, vPar, vPerp);

  auto constexpr t0 = 0_s;
  auto constexpr t1 = 1_s;
  auto constexpr dt = 1_ms;
  auto constexpr n = long((t1 - t0) / dt) + 1;

  auto arr = std::make_unique<std::array<std::array<double, 4>, n>>();
  auto& positions = *arr;

  for (auto [t, i] = std::tuple{t0, 0}; t < t1; t += dt, ++i) {
    auto const r = h.GetPosition(t).GetCoordinates();

    positions[i][0] = t / 1_s;
    positions[i][1] = r[0] / 1_m;
    positions[i][2] = r[1] / 1_m;
    positions[i][3] = r[2] / 1_m;
  }

  std::cout << positions[n - 2][0] << " " << positions[n - 2][1] << " "
            << positions[n - 2][2] << " " << positions[n - 2][3] << std::endl;

  return EXIT_SUCCESS;
}
