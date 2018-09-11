/**
   @file Particles.h

   Interface to particle properties
 */

#ifndef _include_Particle_h_
#define _include_Particle_h_

#include <array>
#include <cstdint>
#include <iostream>

#include <corsika/units/PhysicalUnits.h>
#include <corsika/particles/GeneratedParticleProperties.inc>

/**
 * @namespace particle
 *
 * The properties of all elementary particles is stored here. The data
 * is taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika::particles {

  /**
   * @function GetMass
   *
   * return mass of particle
   */
  auto constexpr GetMass(Code const p) { return masses[static_cast<uint8_t const>(p)]; }

  auto constexpr GetPDG(Code const p) { return pdg_codes[static_cast<uint8_t const>(p)]; }

  auto constexpr GetElectricChargeNumber(Code const p) {
    return electric_charge[static_cast<uint8_t const>(p)] / 3;
  }

  auto constexpr GetElectricCharge(Code const p) {
    return GetElectricChargeNumber(p) * (corsika::units::constants::e);
  }

  auto const GetName(Code const p) { return names[static_cast<uint8_t const>(p)]; }

  namespace io {

    std::ostream& operator<<(std::ostream& stream, Code const p) {
      stream << GetName(p);
      return stream;
    }

  } // namespace io

} // namespace corsika::particles

// to inject the operator<< into the root namespace
using namespace corsika::particles::io;

#endif
