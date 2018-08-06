#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <Units/units/quantity.hpp>
#include <Units/units/io.hpp>

#include <Units/PhysicalConstants.h>

// define _XeV literals

//namespace corsika {
  namespace phys {
    namespace units {
      namespace literals {
        QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(eV) )
      }
    }
  }
//}

#endif
