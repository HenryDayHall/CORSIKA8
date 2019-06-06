/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;

using namespace corsika::geometry;
using namespace corsika::units::si;

TEST_CASE("four vectors") {

  // this is just needed as a baseline
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  /*
    Test: P2 = E2 - p2 all in [GeV]
    This is the typical HEP application
   */
  SECTION("Energy momentum in hep-units") {

    HEPEnergyType E0 = 10_GeV;
    Vector<hepmomentum_d> P0(rootCS, {10_GeV, 10_GeV, 10_GeV});

    FourVector p0(E0, P0);

    REQUIRE(p0.GetNormSqr() == -200_GeV * 1_GeV);
    REQUIRE(p0.GetNorm() == sqrt(200_GeV * 1_GeV));
  }

  /*
    Check space/time-like
   */
  SECTION("Space/time likeness") {

    HEPEnergyType E0 = 20_GeV;
    Vector<hepmomentum_d> P0(rootCS, {10_GeV, 0_GeV, 0_GeV});
    Vector<hepmomentum_d> P1(rootCS, {10_GeV, 10_GeV, 20_GeV});
    Vector<hepmomentum_d> P2(rootCS, {0_GeV, 20_GeV, 0_GeV});

    FourVector p0(E0, P0);
    FourVector p1(E0, P1);
    FourVector p2(E0, P2);

    CHECK(p0.IsSpacelike());
    CHECK(!p0.IsTimelike());
    // CHECK(!p0.IsPhotonlike());

    CHECK(!p1.IsSpacelike());
    CHECK(p1.IsTimelike());
    // CHECK(!p1.IsPhotonlike());

    CHECK(!p2.IsSpacelike());
    CHECK(!p2.IsTimelike());
    // CHECK(p2.IsPhotonlike());
  }

  /*
    Test: P2 = E2/c2 - p2 with E in [GeV/c] and P in [GeV]
    This requires additional factors of c
   */
  SECTION("Energy momentum in SI-units") {

    auto E1 = 100_GeV / corsika::units::constants::c;
    Vector<hepmomentum_d> P1(rootCS, {10_GeV, 5_GeV, 15_GeV});

    FourVector p1(E1, P1);

    const double check = 100 * 100 - 10 * 10 - 5 * 5 - 15 * 15; // for dummies...

    REQUIRE(p1.GetNormSqr() / 1_GeV / 1_GeV == Approx(check));
    REQUIRE(p1.GetNorm() / 1_GeV == Approx(sqrt(check)));
  }

  /**
    Test: P2 = T2/c2 - r2 with T in [s] and r in [m]
    This requires additional factors of c
   */
  SECTION("Spacetime in SI-units") {

    TimeType T2 = 10_m / corsika::units::constants::c;
    Vector<length_d> P2(rootCS, {10_m, 5_m, 5_m});

    const double check = 10 * 10 - 10 * 10 - 5 * 5 - 5 * 5; // for dummies...

    FourVector p2(T2, P2);

    REQUIRE(p2.GetNormSqr() == check * 1_m * 1_m);
    REQUIRE(p2.GetNorm() == sqrt(abs(check)) * 1_m);
  }

  /**
     Testing the math operators
   */

  SECTION("Operators and comutions") {

    HEPEnergyType E1 = 100_GeV;
    Vector<hepmomentum_d> P1(rootCS, {0_GeV, 0_GeV, 0_GeV});

    HEPEnergyType E2 = 0_GeV;
    Vector<hepmomentum_d> P2(rootCS, {10_GeV, 0_GeV, 0_GeV});

    FourVector p1(E1, P1);
    const FourVector p2(E2, P2);

    REQUIRE(p1.GetNorm() / 1_GeV == Approx(100.));
    REQUIRE(p2.GetNorm() / 1_GeV == Approx(10.));

    SECTION("product") {
      FourVector p3 = p1 + p2;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(sqrt(100. * 100. - 100.)));
      p3 -= p2;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(100.));
      REQUIRE(p1.GetNorm() / 1_GeV == Approx(100.));
      REQUIRE(p2.GetNorm() / 1_GeV == Approx(10.));
    }

    SECTION("difference") {
      FourVector p3 = p1 - p2;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(sqrt(100. * 100. - 100.)));
      p3 += p2;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(100.));
      REQUIRE(p1.GetNorm() / 1_GeV == Approx(100.));
      REQUIRE(p2.GetNorm() / 1_GeV == Approx(10.));
    }

    SECTION("scale") {
      double s = 10;
      FourVector p3 = p1 * s;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(sqrt(100. * 100. * s * s)));
      p3 /= 10;
      REQUIRE(p3.GetNorm() / 1_GeV == Approx(sqrt(100. * 100.)));
      REQUIRE(p1.GetNorm() / 1_GeV == Approx(100.));
      REQUIRE(p2.GetNorm() / 1_GeV == Approx(10.));
    }
  }

  /**
     The FourVector class can be used with reference template
     arguments. In this configuration it does not hold any data
     itself, but rather just refers to data located elsewhere. Thus,
     it merely provides the physical/mathematical wrapper around the
     data.
   */

  SECTION("Use as wrapper") {

    TimeType T = 10_m / corsika::units::constants::c;
    Vector<length_d> P(rootCS, {10_m, 5_m, 5_m});

    const TimeType T_c = 10_m / corsika::units::constants::c;
    const Vector<length_d> P_c(rootCS, {10_m, 5_m, 5_m});

    // FourVector<TimeType&, Vector<length_d>&> p0(T_c, P_c); // this does not compile,
    // and it shoudn't!
    FourVector<TimeType&, Vector<length_d>&> p1(T, P);
    FourVector<const TimeType&, const Vector<length_d>&> p2(T, P);
    FourVector<const TimeType&, const Vector<length_d>&> p3(T_c, P_c);

    std::cout << type_id_with_cvr<decltype(p1)>().pretty_name() << std::endl;
    std::cout << type_id_with_cvr<decltype(p2)>().pretty_name() << std::endl;
    std::cout << type_id_with_cvr<decltype(p3)>().pretty_name() << std::endl;

    p1 *= 10;
    // p2 *= 10; // this does not compile, and it shoudn't !
    // p3 *= 10; // this does not compile, and it shoudn't !!

    const double check = 10 * 10 - 10 * 10 - 5 * 5 - 5 * 5; // for dummies...
    REQUIRE(p1.GetNormSqr() / (1_m * 1_m) == Approx(10. * 10. * check));
    REQUIRE(p2.GetNorm() / 1_m == Approx(10 * sqrt(abs(check))));
    REQUIRE(p3.GetNorm() / 1_m == Approx(sqrt(abs(check))));
  }
}
