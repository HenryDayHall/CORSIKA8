#include <fwk/CoordinateSystem.h>
#include <fwk/Helix.h>
#include <fwk/PhysicalUnits.h>
#include <fwk/Point.h>
#include <fwk/Vector.h>

#include <array>
#include <cstdlib>
#include <iostream>

using namespace phys::units;
using namespace phys::units::io;       // support stream << unit
using namespace phys::units::literals; // support unit literals like 5_m;

int main() {
  fwk::CoordinateSystem root;

  fwk::Point const r0(root, {0_m, 0_m, 0_m});
  auto const omegaC = 2 * M_PI * 1_Hz;
  fwk::Vector<speed_d> vPar(root, {0_m / second, 0_m / second, 10_cm / second});
  fwk::Vector<speed_d> vPerp(root, {1_m / second, 0_m / second, 0_m / second});

  fwk::Helix h(r0, omegaC, vPar, vPerp);

  auto constexpr t0 = 0_s;
  auto constexpr t1 = 1_s;
  auto constexpr dt = 1_us;
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
