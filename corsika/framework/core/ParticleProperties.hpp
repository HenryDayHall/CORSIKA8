/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @file ParticleProperties.hpp

   Interface to particle properties
 */

#pragma once

#include <array>
#include <cstdint>
#include <cmath>
#include <iosfwd>
#include <string_view>
#include <type_traits>

#include <corsika/framework/core/PhysicalUnits.hpp>

/**
 *
 * The properties of all elementary particles is stored here. The data
 * are taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika {
  /**
   * @enum Code
   * The Code enum is the actual place to define CORSIKA 8 particle codes.
   */
  enum class Code : int16_t;
  enum class PDGCode : int32_t;
  using CodeIntType = std::underlying_type<Code>::type;
  using PDGCodeType = std::underlying_type<PDGCode>::type;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr get_charge_number(Code);     //!< electric charge in units of e
  ElectricChargeType constexpr get_charge(Code); //!< electric charge
  HEPMassType constexpr get_mass(Code);          //!< mass

  //! Particle code according to PDG, "Monte Carlo Particle Numbering Scheme"
  PDGCode constexpr get_PDG(Code);
  constexpr std::string_view get_name(Code); //!< name of the particle as string
  TimeType constexpr get_lifetime(Code);       //!< lifetime

  //! true iff the particle is a hard-coded nucleus or Code::Nucleus
  bool constexpr is_nucleus(Code);
  bool constexpr is_hadron(Code);    //!< true iff particle is hadron
  bool constexpr is_em(Code);        //!< true iff particle is electron, positron or gamma
  bool constexpr is_muon(Code);      //!< true iff particle is mu+ or mu-
  bool constexpr is_neutrino(Code);  //!< true iff particle is (anti-) neutrino
  int constexpr get_nucleus_A(Code); //!< returns A for hard-coded nucleus, otherwise 0
  int constexpr get_nucleus_Z(Code); //!< returns Z for hard-coded nucleus, otherwise 0

  //! returns mass of (A,Z) nucleus, disregarding binding energy
  inline HEPMassType nucleus_mass(int, int);

  //! convert PDG code to CORSIKA 8 internal code
  inline Code convert_from_PDG(PDGCode);

  //! the output stream operator for human-readable particle codes
  inline std::ostream& operator<<(std::ostream&, corsika::Code);
} // namespace corsika

// data arrays, etc.
#include <corsika/framework/core/GeneratedParticleProperties.inc>

#include <corsika/detail/framework/core/ParticleProperties.inl>

// constants in namespaces-like static classes
#include <corsika/framework/core/GeneratedParticleClasses.inc>
