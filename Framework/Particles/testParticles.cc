#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <Units/PhysicalUnits.h>

#include <Particles/Particles.h>

using namespace phys::units;
using namespace phys::units::literals;

using namespace ParticleProperties;

TEST_CASE( "Particles", "[Particles]" )
{  
  SECTION( "Types" )
    {
      REQUIRE( Electron::GetType()==InternalParticleCode::Electron );
    }

    SECTION( "Data" )
    {
      REQUIRE( Electron::GetMass()/0.511_MeV==Approx(1) );
      REQUIRE( Electron::GetMass()/GetMass(InternalParticleCode::Electron)==Approx(1) );
      REQUIRE( Electron::GetCharge()/phys::units::e==Approx(-1) );
      REQUIRE( Positron::GetCharge()/phys::units::e==Approx(+1) );
      REQUIRE( GetElectricCharge(Positron::GetAntiParticle())/phys::units::e==Approx(-1) );
      REQUIRE( Electron::GetName() == "e-" );
    }

}
