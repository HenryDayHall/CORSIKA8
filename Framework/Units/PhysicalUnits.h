/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

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
 * Add new units and types we need. Units are compile-time. Literals are
 * used for optimal coding style.
 *
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

  /// defining cross section as area
  using sigma_d = phys::units::area_d;

  // constexpr quantity<area_d> barn{Rep(1e-28L) * square(meter)};

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

  namespace detail {
    template <int N, typename T>
    auto constexpr static_pow([[maybe_unused]] T x) {
      if constexpr (N == 0) {
        return 1;
      } else if constexpr (N > 0) {
        return x * static_pow<N - 1, T>(x);
      } else {
        return 1 / static_pow<-N, T>(x);
      }
    }
  } // namespace detail

  template <typename DimFrom, typename DimTo>
  auto constexpr ConversionFactorHEPToSI() {
    static_assert(DimFrom::dim1 == 0 && DimFrom::dim2 == 0 && DimFrom::dim3 == 0 &&
                      DimFrom::dim4 == 0 && DimFrom::dim5 == 0 && DimFrom::dim6 == 0 &&
                      DimFrom::dim7 == 0,
                  "must be a pure HEP type");

    static_assert(
        DimTo::dim4 == 0 && DimTo::dim5 == 0 && DimTo::dim6 == 0 && DimTo::dim7 == 0,
        "conversion possible only into L, M, T dimensions");

    int constexpr e = DimFrom::dim8; // HEP dim.

    int constexpr l = DimTo::dim1; // SI length dim.
    int constexpr m = DimTo::dim2; // SI mass dim.
    int constexpr t = DimTo::dim3; // SI time dim.

    int constexpr p = m;
    int constexpr q = -m - t;
    static_assert(q == l + e - 2 * m, "HEP/SI dimension mismatch!");

    using namespace detail;
    return static_pow<-e>(corsika::units::constants::hBarC) *
           static_pow<p>(corsika::units::constants::hBar) *
           static_pow<q>(corsika::units::constants::c);
  }

  template <typename DimFrom>
  auto constexpr ConversionFactorSIToHEP() {
    static_assert(DimFrom::dim4 == 0 && DimFrom::dim5 == 0 && DimFrom::dim6 == 0 &&
                      DimFrom::dim7 == 0 && DimFrom::dim8 == 0,
                  "must be pure L, M, T type");

    int constexpr l = DimFrom::dim1; // SI length dim.
    int constexpr m = DimFrom::dim2; // SI mass dim.
    int constexpr t = DimFrom::dim3; // SI time dim.

    int constexpr p = -m;
    int constexpr q = m + t;
    int constexpr e = m - t - l;

    using namespace detail;
    return static_pow<e>(corsika::units::constants::hBarC) *
           static_pow<p>(corsika::units::constants::hBar) *
           static_pow<q>(corsika::units::constants::c);
  }

  template <typename DimTo, typename DimFrom>
  auto constexpr ConvertHEPToSI(quantity<DimFrom> q) {
    return ConversionFactorHEPToSI<DimFrom, DimTo>() * q;
  }

  template <typename DimFrom>
  auto constexpr ConvertSIToHEP(quantity<DimFrom> q) {
    return ConversionFactorSIToHEP<DimFrom>() * q;
  }
} // end namespace corsika::units::si

/**
 * @file PhysicalUnits
 *
 */

namespace phys {
  namespace units {
    namespace literals {

      /**
       * Define new _XeV literals, alowing 10_GeV in the code.
       * Define new _barn literal
       */

      QUANTITY_DEFINE_SCALING_LITERALS(eV, hepenergy_d, 1)

      QUANTITY_DEFINE_SCALING_LITERALS(b, corsika::units::si::sigma_d,
                                       magnitude(corsika::units::constants::barn))

    } // namespace literals
  }   // namespace units
} // namespace phys

#endif
