#ifndef _INCLUDE_PHYSICAL_CONSTANTS_H_
#define _INCLUDE_PHYSICAL_CONSTANTS_H_

#include "Units/PhysicalUnits.h"

namespace phys {
  namespace units {

    // electronvolt
    constexpr quantity< energy_d >  eV { Rep( 1.60217733e-19L ) * joule };
    
    // elementary charge
    constexpr quantity< electric_charge_d >
      e { Rep( 1.602176462e-19L ) * coulomb };
    
    // Planck constant
    constexpr quantity< dimensions< 2, 1, -1 > >
      h { Rep( 6.62606876e-34L ) * joule * second };
    
    // speed of light in a vacuum
    constexpr quantity< speed_d >   c { Rep( 299792458L ) * meter / second };
    
  }
} 

#endif 

