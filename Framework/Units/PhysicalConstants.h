/**
 * \file PhysicalConstants
 *
 * \brief   Several physical constants.
 * \author  Michael S. Kenniston, Martin Moene
 * \date    7 September 2013
 * \since   0.4
 *
 * Copyright 2013 Universiteit Leiden. All rights reserved.
 *
 * Copyright (c) 2001 by Michael S. Kenniston.  For the most
 * recent version check www.xnet.com/~msk/quantity.  Permission is granted
 * to use this code without restriction so long as this copyright
 * notice appears in all source files.
 *
 * This code is provided as-is, with no warrantee of correctness.
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *
 *
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

  constexpr quantity<dimensions<1, 0, 0, 0, 0, 0, 0, 1>> hBarC{
      Rep(1.973'269'78e-7L) * electronvolt * meter}; // from RPP 2018

  // unified atomic mass unit
  constexpr quantity<mass_d> u{Rep(1.6605402e-27L) * kilogram};

  // etc.

} // namespace corsika::units::constants

#endif // PHYS_UNITS_PHYSICAL_CONSTANTS_HPP_INCLUDED
