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

#include <corsika/units/PhysicalUnits.h>

#include <array>
#include <sstream>

using namespace corsika;
using namespace corsika::units::si;

TEST_CASE("PhysicalUnits", "[Units]") {

  SECTION("Consistency") {
    REQUIRE(1_m / 1_m == Approx(1));
    // REQUIRE_FALSE( 1_m/1_s == 1 ); // static assert
  }

  SECTION("Constructors") {
    [[maybe_unused]] auto E1 = 10_GeV;
    REQUIRE(E1 == 10_GeV);

    LengthType l1 = 10_nm;
    [[maybe_unused]] auto l2 = l1;
    REQUIRE(l2 == l1);

    LengthType arr0[5];
    arr0[0] = 5_m;

    [[maybe_unused]] LengthType arr1[2] = {{1_mm}, {2_cm}};

    [[maybe_unused]] std::array<HEPEnergyType, 4> arr2; // empty array

    [[maybe_unused]] std::array<HEPEnergyType, 4> arr3 = {1_GeV, 1_eV, 5_MeV};

    auto p1 = 10_s * newton;
    REQUIRE(p1 == 10_s * newton);
  }

  SECTION("Powers in literal units") {
    REQUIRE(1_s / 1_ds == Approx(1e1));
    REQUIRE(1_m / 1_cm == Approx(1e2));
    REQUIRE(1_m / 1_mm == Approx(1e3));
    REQUIRE(1_V / 1_uV == Approx(1e6));
    REQUIRE(1_s / 1_ns == Approx(1e9));
    REQUIRE(1_eV / 1_peV == Approx(1e12));
    REQUIRE(1_A / 1_fA == Approx(1e15));
    REQUIRE(1_mol / 1_amol == Approx(1e18));
    REQUIRE(1_K / 1_zK == Approx(1e21));
    REQUIRE(1_K / 1_yK == Approx(1e24));
    REQUIRE(1_b / 1_mb == Approx(1e3));

    REQUIRE(1_A / 1_hA == Approx(1e-2));
    REQUIRE(1_m / 1_km == Approx(1e-3));
    REQUIRE(1_m / 1_Mm == Approx(1e-6));
    REQUIRE(1_V / 1_GV == Approx(1e-9));
    REQUIRE(1_s / 1_Ts == Approx(1e-12));
    REQUIRE(1_eV / 1_PeV == Approx(1e-15));
    REQUIRE(1_A / 1_EA == Approx(1e-18));
    REQUIRE(1_K / 1_ZK == Approx(1e-21));
    REQUIRE(1_mol / 1_Ymol == Approx(1e-24));

    REQUIRE(std::min(1_A, 2_A) == 1_A);
  }

  SECTION("Powers and units") {
    REQUIRE(1 * ampere / 1_A == Approx(1e0));
    REQUIRE(mega * bar / bar == Approx(1e6));
  }

  SECTION("Formulas") {
    const HEPEnergyType E2 = 20_GeV * 2;
    REQUIRE(E2 == 40_GeV);
    REQUIRE(E2 / 1_GeV == Approx(40));

    const MassType m = 1_kg;
    const SpeedType v = 1_m / 1_s;
    REQUIRE(m * v == 1_s * newton);

    const double lgE = log10(E2 / 1_GeV);
    REQUIRE(lgE == Approx(log10(40.)));

    const auto E3 = E2 + 100_GeV + pow(10, lgE) * 1_GeV;
    REQUIRE(E3 == 180_GeV);
  }

  SECTION("Output") {
    {
      const HEPEnergyType E = 5_eV;
      std::stringstream stream;
      stream << E;
      REQUIRE(stream.str() == std::string("5 eV"));
    }
    {
      const HEPEnergyType E = 5_EeV;
      std::stringstream stream;
      stream << E;
      REQUIRE(stream.str() == std::string("5e+18 eV"));
    }
  }

  SECTION("Special") {

    const LengthType farAway = std::numeric_limits<double>::infinity() * meter;
    REQUIRE(farAway > 100000_m);
    REQUIRE_FALSE(farAway < 1e19 * meter);
  }

  SECTION("static_pow") {
    using namespace corsika::units::si::detail;
    double x = 235.7913;
    REQUIRE(1 == static_pow<0, double>(x));
    REQUIRE(x == static_pow<1, double>(x));
    REQUIRE(x * x == static_pow<2, double>(x));
    REQUIRE(1 / x == static_pow<-1, double>(x));
    REQUIRE(1 / x / x == static_pow<-2, double>(x));
  }

  SECTION("HEP/SI conversion") {
    auto const invEnergy = 1 / 197.326978_MeV; // should be convertible to length or time

    LengthType const length = ConvertHEPToSI<LengthType::dimension_type>(invEnergy);
    REQUIRE((length / 1_fm) == Approx(1));

    TimeType const time = ConvertHEPToSI<TimeType::dimension_type>(invEnergy);
    REQUIRE((time / (1_fm / corsika::units::constants::c)) == Approx(1));

    auto const protonMass = 938.272'081'3_MeV; // convertible to mass or SI energy
    MassType protonMassSI = ConvertHEPToSI<MassType::dimension_type>(protonMass);
    REQUIRE((protonMassSI / 1.672'621'898e-27_kg) == Approx(1));
    REQUIRE((protonMassSI / (1.007'276 * corsika::units::constants::u)) == Approx(1));
  }

  SECTION("SI/HEP conversion") {
    REQUIRE(ConvertSIToHEP(units::constants::c) == Approx(1));
    REQUIRE(ConvertSIToHEP(units::constants::hBar) == Approx(1));

    {
      auto const invLength = 1 / 197.326978_fm; // should be convertible to HEPEnergy
      HEPEnergyType const energy = ConvertSIToHEP(invLength);
      REQUIRE(energy / 1_MeV == Approx(1));
    }

    REQUIRE(ConvertSIToHEP(6.5823e-25_s) * 1_GeV == Approx(1).epsilon(1e-4));

    REQUIRE(ConvertSIToHEP(3.8938e-32 * meter * meter) * 1_GeV * 1_GeV ==
            Approx(1).epsilon(1e-4));
  }
}
