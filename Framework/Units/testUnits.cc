#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <ThirdParty/catch2/catch.hpp>

#include <Units/PhysicalUnits.h>

using namespace phys::units;
using namespace phys::units::io;
using namespace phys::units::literals;

TEST_CASE( "PhysicalUnits", "[Units]" ) {  
  REQUIRE( 1_m/1_m == 1 );
}
