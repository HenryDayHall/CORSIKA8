#ifndef _include_PhysicalUnits_h_
#define _include_PhysicalUnits_h_

#include <corsika/units/PhysicalConstants.h>

#include <phys/units/io.hpp>
#include <phys/units/quantity.hpp>

/*
  It is essentially a bug of the phys/units package to define the
  operator<< not in the same namespace as the types it is working
  on. This breaks ADL (argument-dependent lookup). Here we "fix" this:
 */
namespace phys::units {
  // using namespace phys::units::io;
  using phys::units::io::operator<<;
} // namespace phys::units

/**
 * @file PhysicalUnits
 *
 * Add new units and types we need. Units are compile-time. We support
 * different system of units in parallel. Literals are used for
 * optimal coding style.
 *
 */

namespace corsika::units::hep {
  using namespace phys::units;
  using namespace phys::units::literals;
  using namespace phys::units::io;

  /// defining HEP energy, mass, momentum
  using energy_hep_d = phys::units::energy_d;
  constexpr phys::units::quantity<energy_hep_d> GeV{corsika::units::constants::eV};
  // corsika::units::constants::e / phys::units::coulomb * phys::units::joule };

  using MassType = phys::units::quantity<energy_hep_d, double>;
  using MomentumType = phys::units::quantity<energy_hep_d, double>;
  using EnergyType = phys::units::quantity<energy_hep_d, double>;

} // namespace corsika::units::hep

namespace corsika::units::si {
  using namespace phys::units;
  using namespace phys::units::literals;
  using namespace phys::units::io;
  using phys::units::io::operator<<;

  /// defining momentum you suckers
  /// dimensions, i.e. composition in base SI dimensions
  using momentum_d = phys::units::dimensions<1, 1, -1>;
  // defining the unit of momentum, so far newton-meter, maybe go to HEP?
  constexpr phys::units::quantity<momentum_d> newton_second{
      phys::units::meter * phys::units::kilogram / phys::units::second};

  /// defining cross section
  using sigma_d = phys::units::dimensions<2, 0, 0>;
  constexpr phys::units::quantity<sigma_d> barn{phys::units::Rep(1.e-28L) *
                                                phys::units::meter * phys::units::meter};

  /// add the unit-types
  using LengthType = phys::units::quantity<phys::units::length_d, double>;
  using TimeType = phys::units::quantity<phys::units::time_interval_d, double>;
  using SpeedType = phys::units::quantity<phys::units::speed_d, double>;
  using FrequencyType = phys::units::quantity<phys::units::frequency_d, double>;
  using ElectricChargeType =
      phys::units::quantity<phys::units::electric_charge_d, double>;
  using EnergyType = phys::units::quantity<phys::units::energy_d, double>;
  using MassType = phys::units::quantity<phys::units::mass_d, double>;
  using MassDensityType = phys::units::quantity<phys::units::mass_density_d, double>;
  using GrammageType = phys::units::quantity<phys::units::dimensions<-2, 1, 0>, double>;
  using MomentumType = phys::units::quantity<momentum_d, double>;
  using CrossSectionType = phys::units::quantity<area_d, double>;
  using InverseLengthType =
      phys::units::quantity<phys::units::dimensions<-1, 0, 0>, double>;
  using InverseTimeType =
      phys::units::quantity<phys::units::dimensions<0, 0, -1>, double>;
  using InverseGrammageType =
      phys::units::quantity<phys::units::dimensions<2, -1, 0>, double>;
} // end namespace corsika::units::si

/**
 * @file PhysicalUnits
 *
 * Define _XeV literals, alowing 10_GeV in the code.
 * Define _meter literal
 * Define _barn literal
 * Define _newton_second literal for SI momenta
 */

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, energy_d,
                                       magnitude(corsika::units::constants::eV))

      //      QUANTITY_DEFINE_SCALING_LITERALS(barn, corsika::units::si::area_d,
      //                             magnitude(corsika::units::si::constants::barn))

      QUANTITY_DEFINE_SCALING_LITERALS(barn, corsika::units::si::sigma_d,
                                       magnitude(corsika::units::constants::barn))

      QUANTITY_DEFINE_SCALING_LITERALS(meter, length_d,
                                       magnitude(corsika::units::constants::meter))

      QUANTITY_DEFINE_SCALING_LITERALS(Ns, corsika::units::si::momentum_d,
                                       magnitude(1_m * 1_kg / 1_s))

    } // namespace literals
  }   // namespace units
} // namespace phys

#endif
