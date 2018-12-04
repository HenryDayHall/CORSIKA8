
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @file Particles.h

   Interface to particle properties
 */

#ifndef _include_corsika_particles_ParticleProperties_h_
#define _include_corsika_particles_ParticleProperties_h_

#include <array>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>


/**
 * @namespace particle
 *
 * The properties of all elementary particles is stored here. The data
 * is taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika::particles {

  using corsika::units::si::second;

  enum class Code : int16_t;

  using PDGCodeType = int16_t;
  using CodeIntType = std::underlying_type<Code>::type;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr GetElectricChargeNumber(Code const);
  corsika::units::si::ElectricChargeType constexpr GetElectricCharge(Code const);
  corsika::units::si::MassType constexpr GetMass(Code const);
  PDGCodeType constexpr GetPDG(Code const);
  constexpr std::string const& GetName(Code const);
  corsika::units::si::TimeType constexpr GetLifetime(Code const);

#include <corsika/particles/GeneratedParticleProperties.inc>

  /*!
   * returns mass of particle
   */
  corsika::units::si::MassType constexpr GetMass(Code const p) {
    return masses[static_cast<CodeIntType const>(p)];
  }

  PDGCodeType constexpr GetPDG(Code const p) {
    return pdg_codes[static_cast<CodeIntType const>(p)];
  }

  /*!
   * returns electric charge of particle / (e/3).
   */
  int16_t constexpr GetElectricChargeNumber(Code const p) {
    return electric_charges[static_cast<CodeIntType const>(p)];
  }

  corsika::units::si::ElectricChargeType constexpr GetElectricCharge(Code const p) {
    return GetElectricChargeNumber(p) * (corsika::units::si::constants::e / 3.);
  }

  constexpr std::string const& GetName(Code const p) {
    return names[static_cast<CodeIntType const>(p)];
  }

  corsika::units::si::TimeType constexpr GetLifetime(Code const p) {
    return lifetime[static_cast<CodeIntType const>(p)];
  }

  namespace io {

    std::ostream& operator<<(std::ostream& stream, Code const p);

  } // namespace io

} // namespace corsika::particles

// to inject the operator<< into the root namespace
using namespace corsika::particles::io;

#endif
