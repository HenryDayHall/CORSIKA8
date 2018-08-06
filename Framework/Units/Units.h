#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include "boost/units/quantity.hpp"
#include "boost/units/io.hpp"
#include "boost/units/physical_constants.hpp"

//#include <cstdlib>
//#include <iostream>

// define _XeV literals

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(eV) )
    }
  }
}

#endif
