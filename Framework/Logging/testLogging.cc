#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <Logging/Logger.h>

TEST_CASE( "Logging", "[Logging]" )
{  
  SECTION( "sectionOne" )
    {
      REQUIRE( 1/1 == 1 );
    }
  
  SECTION( "sectionTwo" )
    {
      REQUIRE_FALSE( 1/1 == 2 );
    }

  SECTION( "sectionThree" )
    {
      REQUIRE( 1/1 == 2 );
    }
}
