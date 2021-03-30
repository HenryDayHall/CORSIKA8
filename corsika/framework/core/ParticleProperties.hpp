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
#include <unordered_map>

#include <corsika/framework/core/PhysicalUnits.hpp>


namespace corsika {

/**
   @defgroup Particles Particle Properties

   The properties of all particles are saved in static and flat
   arrays. There is a enum corsika::Code to identify each
   particles, and each individual particles has its own static class,
   which can be used to retrieve its physical properties.

   The properties of all elementary particles are accessible here. The data
   are taken from the Pythia ParticleData.xml file. 

   Particle data can be accessed via global function in namespace corsika, or via 
   static classes for each particle type. These classes all have the interface (example for 
   the class corsika::Electron):

   @code{.cpp}
     static constexpr Code code{Code::Electron};
     static constexpr Code anti_code{Code::Positron};
     static constexpr HEPMassType mass{corsika::get_mass(code)};
     static constexpr ElectricChargeType charge{corsika::get_charge(code)};
     static constexpr int charge_number{corsika::get_charge_number(code)};
     static constexpr std::string_view name{corsika::get_name(code)};
     static constexpr bool is_nucleus{corsika::is_nucleus(code)};
   @endcode

   The names, relations and properties of all particles known to CORSIKA 8 are listed below. 

   @addtogroup Particles
   @{
 */

  /** The Code enum is the actual place to define CORSIKA 8 particle codes. */
  enum class Code : int16_t;
  
  /** Specifically for PDG ids */
  enum class PDGCode : int32_t;
  
  using CodeIntType = std::underlying_type<Code>::type;
  using PDGCodeType = std::underlying_type<PDGCode>::type;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr get_charge_number(Code const);     //!< electric charge in units of e
  ElectricChargeType constexpr get_charge(Code const); //!< electric charge
  HEPMassType constexpr get_mass(Code const);          //!< mass
  HEPEnergyType constexpr get_energy_threshold(
      Code const); //!< get energy threshold below which the particle is discarded, by
                   //!< default set to particle mass
  void constexpr set_energy_threshold(
      Code const, HEPEnergyType const); //!< set energy threshold below which the particle
                                        //!< is discarded

  inline void set_energy_threshold(std::pair<Code const, HEPEnergyType const> p) {
    set_energy_threshold(p.first, p.second);
  }
  inline void set_energy_thresholds(
      std::unordered_map<Code const, HEPEnergyType const> const& eCuts) {
    for (auto v : eCuts) set_energy_threshold(v);
  }

  //! Particle code according to PDG, "Monte Carlo Particle Numbering Scheme"
  PDGCode constexpr get_PDG(Code const);
  std::string_view constexpr get_name(Code const); //!< name of the particle as string
  TimeType constexpr get_lifetime(Code const);     //!< lifetime

  //! true iff the particle is a hard-coded nucleus or Code::Nucleus
  bool constexpr is_nucleus(Code const);
  bool constexpr is_hadron(Code const); //!< true iff particle is hadron
  bool constexpr is_em(Code const); //!< true iff particle is electron, positron or gamma
  bool constexpr is_muon(Code const);     //!< true iff particle is mu+ or mu-
  bool constexpr is_neutrino(Code const); //!< true iff particle is (anti-) neutrino
  int constexpr get_nucleus_A(
      Code const); //!< returns A for hard-coded nucleus, otherwise 0
  int constexpr get_nucleus_Z(
      Code const); //!< returns Z for hard-coded nucleus, otherwise 0

  //! returns mass of (A,Z) nucleus, disregarding binding energy
  HEPMassType get_nucleus_mass(unsigned int const, unsigned int const);

  //! convert PDG code to CORSIKA 8 internal code
  Code convert_from_PDG(PDGCode const);

  std::initializer_list<Code> constexpr get_all_particles();

  //! the output stream operator for human-readable particle codes
  std::ostream& operator<<(std::ostream&, corsika::Code);

  /** @}*/

} // namespace corsika

// data arrays, etc., as generated automatically
#include <corsika/framework/core/GeneratedParticleProperties.inc>

#include <corsika/detail/framework/core/ParticleProperties.inl>

// constants in namespaces-like static classes, generated automatically
#include <corsika/framework/core/GeneratedParticleClasses.inc>

