#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <ThirdParty/phys/units/quantity.hpp>
#include <ThirdParty/phys/units/io.hpp>
#include <ThirdParty/phys/units/physical_constants.hpp>

/**
   /file PhysicalUnits
   
   Define _XeV literals, alowing 10_GeV in the code.     
*/

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(eV) )
    }
  }
}

#endif

