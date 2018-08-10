#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <phys/units/quantity.hpp>
#include <phys/units/io.hpp>
#include <phys/units/physical_constants.hpp>

/**
   @file PhysicalUnits
   
   Define _XeV literals, alowing 10_GeV in the code.     
*/

using namespace phys::units::io;
using namespace phys::units::literals;

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(eV) )
    }
  }
}

#endif

