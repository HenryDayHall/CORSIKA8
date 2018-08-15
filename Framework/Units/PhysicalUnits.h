#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <phys/units/quantity.hpp>
#include <phys/units/io.hpp>
#include <phys/units/physical_constants.hpp>

/**
   @file PhysicalUnits
   
   Define _XeV literals, alowing 10_GeV in the code.     
*/

/*using namespace phys::units::io;
using namespace phys::units::literals;*/

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(eV) )
    }
  }
}

using Length = phys::units::quantity<phys::units::length_d, double>;
using Time = phys::units::quantity<phys::units::time_interval_d, double>;
using Speed = phys::units::quantity<phys::units::speed_d, double>;
using Frequency = phys::units::quantity<phys::units::frequency_d, double>;
using ElectricCharge = phys::units::quantity<phys::units::electric_charge_d, double>;

#endif

