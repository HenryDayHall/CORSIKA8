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
/*
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
*/
namespace corsika::units::si {
  using namespace phys::units;
  using namespace phys::units::literals;
  using namespace phys::units::io;
  using phys::units::io::operator<<;

  /// defining momentum you suckers
  /// dimensions, i.e. composition in base SI dimensions
  using hepmomentum_d = phys::units::hepenergy_d;
  using hepmass_d = phys::units::hepenergy_d;

  /// defining cross section
  using sigma_d = phys::units::area_d;

  /// add the unit-types
  using LengthType = phys::units::quantity<phys::units::length_d, double>;
  using TimeType = phys::units::quantity<phys::units::time_interval_d, double>;
  using SpeedType = phys::units::quantity<phys::units::speed_d, double>;
  using FrequencyType = phys::units::quantity<phys::units::frequency_d, double>;
  using ElectricChargeType =
      phys::units::quantity<phys::units::electric_charge_d, double>;
  using HEPEnergyType = phys::units::quantity<phys::units::hepenergy_d, double>;
  using MassType = phys::units::quantity<phys::units::mass_d, double>;
  using HEPMassType = phys::units::quantity<hepmass_d, double>;
  using MassDensityType = phys::units::quantity<phys::units::mass_density_d, double>;
  using GrammageType = phys::units::quantity<phys::units::dimensions<-2, 1, 0>, double>;
  using HEPMomentumType = phys::units::quantity<hepmomentum_d, double>;
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
 * Define _barn literal
 */

namespace phys {
  namespace units {
    namespace literals {
      QUANTITY_DEFINE_SCALING_LITERALS(eV, hepenergy_d, 1)

      QUANTITY_DEFINE_SCALING_LITERALS(barn, corsika::units::si::sigma_d,
                                       magnitude(corsika::units::constants::barn))

      // QUANTITY_DEFINE_SCALING_LITERALS(Ns, corsika::units::si::momentum_d,
      //                                 magnitude(1_m * 1_kg / 1_s))

    } // namespace literals
  }   // namespace units
} // namespace phys

#endif
