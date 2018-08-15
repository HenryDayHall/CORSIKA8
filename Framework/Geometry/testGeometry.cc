#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <Units/PhysicalUnits.h>

using namespace phys::units;
using namespace phys::units::literals;

TEST_CASE( "PhysicalUnits", "[Units]" )
{  
  SECTION( "sectionOne" )
    {
      REQUIRE( 1_m/1_m == 1 );
    }
  
  SECTION( "sectionTwo" )
    {
      REQUIRE_FALSE( 1_m/1_m == 2 );
    }

  SECTION( "sectionThree" )
    {
      REQUIRE( 1_s/1_s == 2 );
    }
}
