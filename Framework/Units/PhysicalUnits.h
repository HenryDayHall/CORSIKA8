#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <fwk/PhysicalConstants.h>

#include <phys/units/io.hpp>
#include <phys/units/quantity.hpp>

/**
 * @file PhysicalUnits
 *
 * Define _XeV literals, alowing 10_GeV in the code.
 */

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d, magnitude(constants::eV))
    }
  } // namespace units
} // namespace phys

namespace fwk {

  using namespace phys::units;
  //namespace literals = phys::units::literals;    
  
  //namespace quantity {
    using Length = phys::units::quantity<phys::units::length_d, double>;
    using Time = phys::units::quantity<phys::units::time_interval_d, double>;
    using Speed = phys::units::quantity<phys::units::speed_d, double>;
    using Frequency = phys::units::quantity<phys::units::frequency_d, double>;
    using ElectricCharge = phys::units::quantity<phys::units::electric_charge_d, double>;
    using Energy = phys::units::quantity<phys::units::energy_d, double>;
    //} // namespace quantity

} // end namespace fwk

// we want to call the operator<< without namespace... I think
using namespace phys::units::io;

#endif
