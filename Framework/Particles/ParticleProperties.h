
/*
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
#include <iosfwd>
#include <type_traits>

#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>

/**
 *
 * The properties of all elementary particles is stored here. The data
 * is taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika::particles {

  /**
   * @enum Code
   * The Code enum is the actual place to define CORSIKA 8 particle codes.
   */
  enum class Code : int16_t;
  using CodeIntType = std::underlying_type<Code>::type;
  using PDGCodeType = int32_t;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr GetElectricChargeNumber(Code const);
  corsika::units::si::ElectricChargeType constexpr GetElectricCharge(Code const);
  corsika::units::si::HEPMassType constexpr GetMass(Code const);
  PDGCodeType constexpr GetPDG(Code const);
  constexpr std::string const& GetName(Code const);
  corsika::units::si::TimeType constexpr GetLifetime(Code const);

  bool constexpr IsNucleus(Code const);
  int constexpr GetNucleusA(Code const);
  int constexpr GetNucleusZ(Code const);

#include <corsika/particles/GeneratedParticleProperties.inc>

  /*!
   * returns mass of particle
   */
  corsika::units::si::HEPMassType constexpr GetMass(Code const p) {
    return detail::masses[static_cast<CodeIntType const>(p)];
  }

  PDGCodeType constexpr GetPDG(Code const p) {
    return detail::pdg_codes[static_cast<CodeIntType const>(p)];
  }

  /*!
   * returns electric charge of particle / (e/3).
   */
  int16_t constexpr GetElectricChargeNumber(Code const p) {
    return detail::electric_charges[static_cast<CodeIntType const>(p)];
  }

  corsika::units::si::ElectricChargeType constexpr GetElectricCharge(Code const p) {
    return GetElectricChargeNumber(p) * (corsika::units::constants::e / 3.);
  }

  constexpr std::string const& GetName(Code const p) {
    return detail::names[static_cast<CodeIntType const>(p)];
  }

  corsika::units::si::TimeType constexpr GetLifetime(Code const p) {
    return detail::lifetime[static_cast<CodeIntType const>(p)] *
           corsika::units::si::second;
  }

  bool constexpr IsNucleus(Code const p) {
    return detail::isNucleus[static_cast<CodeIntType const>(p)];
  }

  int constexpr GetNucleusA(Code const p) {
    return detail::nucleusA[static_cast<CodeIntType const>(p)];
  }

  int constexpr GetNucleusZ(Code const p) {
    return detail::nucleusZ[static_cast<CodeIntType const>(p)];
  }

  /**
   * the output operator for particles
   **/

  std::ostream& operator<<(std::ostream& stream, corsika::particles::Code const p);

} // namespace corsika::particles

#endif
