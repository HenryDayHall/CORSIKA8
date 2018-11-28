#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <corsika/units/PhysicalConstants.h>

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
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d,
                                       magnitude(corsika::units::si::constants::eV));

      // phys::units::quantity<energy_d/mass_d> Joule2Kg = c2; // 1_Joule / 1_kg;

    } // namespace literals
  }   // namespace units
}


namespace corsika::units::si {
  using namespace phys::units;
  using namespace phys::units::literals;
  // namespace literals = phys::units::literals;

  using LengthType = phys::units::quantity<phys::units::length_d, double>;
  using TimeType = phys::units::quantity<phys::units::time_interval_d, double>;
  using SpeedType = phys::units::quantity<phys::units::speed_d, double>;
  using FrequencyType = phys::units::quantity<phys::units::frequency_d, double>;
  using ElectricChargeType =
      phys::units::quantity<phys::units::electric_charge_d, double>;
  using EnergyType = phys::units::quantity<phys::units::energy_d, double>;
  using MassType = phys::units::quantity<phys::units::mass_d, double>;

  // defining momentum you suckers
  // dimensions, i.e. composition in base SI dimensions
  using momentum_d                     = phys::units::dimensions< 1, 1, -1 >;
  // defining the unit of momentum, so far newton-meter, maybe go to HEP?
  constexpr phys::units::quantity< momentum_d > newton_second   { meter * kilogram / second };
  // defining the type
  using MomentumType = phys::units::quantity<momentum_d, double>;

  // defining cross section
  using sigma_d      = phys::units::dimensions< 2, 0, 0 >;
  constexpr phys::units::quantity< sigma_d  > barn       {Rep(1.e-28L) * meter * meter};
  using CrossSectionType = phys::units::quantity<sigma_d, double>;
  
} // end namespace corsika::units::si

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(barn, corsika::units::si::sigma_d,
                                       magnitude(corsika::units::si::constants::barn))

      // phys::units::quantity<energy_d/mass_d> Joule2Kg = c2; // 1_Joule / 1_kg;

    } // namespace literals
  }   // namespace units
}

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(newton_second, corsika::units::si::momentum_d,
                                       magnitude(corsika::units::si::newton_second))

      // phys::units::quantity<energy_d/mass_d> Joule2Kg = c2; // 1_Joule / 1_kg;

    } // namespace literals
  }   // namespace units
}

// we want to call the operator<< without namespace... I think
using namespace phys::units::io;

#endif
