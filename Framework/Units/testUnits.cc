#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <fwk/PhysicalUnits.h>

#include <array>

using namespace phys::units;
using namespace phys::units::literals;

TEST_CASE( "PhysicalUnits", "[Units]" ) {  

  SECTION("Consistency") {
    REQUIRE( 1_m/1_m == Approx(1) );
    //REQUIRE_FALSE( 1_m/1_s == 1 ); // static assert
  }

  SECTION("Constructors") {
    auto E1 = 10_GeV;
    REQUIRE( E1==10_GeV );

    quantity<phys::units::length_d> l1 = 10_nm;
    
    quantity<phys::units::length_d> arr0[5];
    arr0[0] = 5_m;
    
    quantity<phys::units::length_d> arr1[2] = { {1_mm}, {2_cm} };

    std::array<quantity<phys::units::energy_d>, 4> arr2; // empty array
    
    std::array<quantity<phys::units::energy_d>, 4> arr3 = { 1_GeV, 1_eV, 5_MeV };
  }
  
  SECTION("Powers in literal units") {
    REQUIRE( 1_s/1_ds == Approx(1e1) );
    REQUIRE( 1_m/1_cm == Approx(1e2) );
    REQUIRE( 1_m/1_mm == Approx(1e3) );
    REQUIRE( 1_V/1_uV == Approx(1e6) );
    REQUIRE( 1_s/1_ns == Approx(1e9) );
    REQUIRE( 1_eV/1_peV == Approx(1e12) );
    REQUIRE( 1_A/1_fA == Approx(1e15) );
    REQUIRE( 1_mol/1_amol == Approx(1e18) );
    REQUIRE( 1_K/1_zK == Approx(1e21) );
    REQUIRE( 1_K/1_yK == Approx(1e24) );

    REQUIRE( 1_A/1_hA == Approx(1e-2) );
    REQUIRE( 1_m/1_km == Approx(1e-3) );
    REQUIRE( 1_m/1_Mm == Approx(1e-6) );
    REQUIRE( 1_V/1_GV == Approx(1e-9) );
    REQUIRE( 1_s/1_Ts == Approx(1e-12) );
    REQUIRE( 1_eV/1_PeV == Approx(1e-15) );
    REQUIRE( 1_A/1_EA == Approx(1e-18) );
    REQUIRE( 1_K/1_ZK == Approx(1e-21) );
    REQUIRE( 1_mol/1_Ymol == Approx(1e-24) );    
  }

  SECTION("Powers and units") {
    REQUIRE( 1 * ampere / 1_A == Approx(1e0) );
    REQUIRE( mega * bar / bar == Approx(1e6) );
  }

  SECTION("Formulas") {
    const quantity<phys::units::energy_d> E2 = 20_GeV * 2;
    REQUIRE( E2==40_GeV );
    
    const double lgE = log10(E2/1_GeV);
    REQUIRE( lgE==Approx(log10(40.)) );

    const auto E3 = E2 + 100_GeV + pow(10, lgE) * 1_GeV;
    REQUIRE( E3==180_GeV );
  }
  
}
