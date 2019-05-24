/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef INCLUDE_PHYSICAL_CONSTANTS_H
#define INCLUDE_PHYSICAL_CONSTANTS_H

#include <phys/units/quantity.hpp>

namespace corsika::units::constants {

  using namespace phys::units;

  // acceleration of free-fall, standard
  constexpr phys::units::quantity<phys::units::acceleration_d> g_sub_n{
      phys::units::Rep(9.80665L) * phys::units::meter /
      phys::units::square(phys::units::second)};

  // Avogadro constant
  constexpr quantity<dimensions<0, 0, 0, 0, 0, -1>> N_sub_A{Rep(6.02214199e+23L) / mole};

  // elementary charge
  constexpr quantity<electric_charge_d> e{Rep(1.6021766208e-19L) * coulomb};

  // electronvolt
  // constexpr quantity<hepenergy_d> eV{e / coulomb * joule};

  // Planck constant
  constexpr quantity<dimensions<2, 1, -1>> h{Rep(6.62606876e-34L) * joule * second};
  constexpr quantity<dimensions<2, 1, -1>> hBar{h / (2 * M_PI)};

  // speed of light in a vacuum
  constexpr quantity<speed_d> c{Rep(299792458L) * meter / second};
  constexpr auto cSquared = c * c;

  // hbar * c
  constexpr quantity<dimensions<1, 0, 0, 0, 0, 0, 0, 1>> hBarC{
      Rep(1.973'269'78e-7L) * electronvolt * meter}; // from RPP 2018

  auto constexpr invGeVsq = 1e-18 / (electronvolt * electronvolt);

  // unified atomic mass unit
  constexpr quantity<mass_d> u{Rep(1.6605402e-27L) * kilogram};

  auto constexpr nucleonMass = 0.5 * (0.93827 + 0.93957) * 1e9 * electronvolt;

  // molar gas constant
  auto constexpr R = Rep(8.314'459'8) * joule / (mole * kelvin);

  // etc.

} // namespace corsika::units::constants

#endif // PHYS_UNITS_PHYSICAL_CONSTANTS_HPP_INCLUDED
