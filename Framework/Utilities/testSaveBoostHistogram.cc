/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>
#include <corsika/utl/SaveBoostHistogram.hpp>

#include <random>

TEST_CASE("SaveHistogram") {
  std::mt19937 rng;
  std::normal_distribution n1{2., 6.};
  std::exponential_distribution e{1.};
  std::uniform_int_distribution u{1, 10};

  auto h =
      boost::histogram::make_histogram(boost::histogram::axis::regular{5, 0, 10, "x"},
                                       boost::histogram::axis::regular{3, 0, 4, "y"},
                                       boost::histogram::axis::category{1, 4, 8});

  for (int i{0}; i < 100'000; ++i) {
    auto const a = n1(rng);
    auto const b = e(rng);
    auto const c = u(rng);

    h(a, b, c);
  }

  corsika::utl::save_hist(h, "hist.npz");
}
