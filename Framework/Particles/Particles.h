/**
   @file Particles.h

   Interface to particle properties
 */

#ifndef _include_Particle_h_
#define _include_Particle_h_

#include <array>
#include <cstdint>
#include <iostream>

#include <fwk/GeneratedParticleProperties.inc>


namespace fwk {

  /**
   * @namespace particle
   * 
   * The properties of all elementary particles is stored here. The data 
   * is taken from the Pythia ParticleData.xml file.
   * 
   */
  
  namespace particle {

    /**
     * @function GetMass
     * 
     * return mass of particle
     */
    auto constexpr GetMass(InternalParticleCode const p)
    {
      return masses[static_cast<uint8_t const>(p)];
    }
    
    auto constexpr GetPDG(InternalParticleCode const p)
    {
      return pdg_codes[static_cast<uint8_t const>(p)];
    }
    
    auto constexpr GetElectricChargeNumber(InternalParticleCode const p)
    {
      return electric_charge[static_cast<uint8_t const>(p)] / 3;
    }
    
    auto constexpr GetElectricCharge(InternalParticleCode const p)
    {
      return GetElectricChargeNumber(p) * (phys::units::e);
    }
    
    auto const GetName(InternalParticleCode const p)
    {
      return names[static_cast<uint8_t const>(p)];
    }
    
    std::ostream& operator<< (std::ostream& stream, InternalParticleCode const p)
      {
	stream << GetName(p);
	return stream;
      }

  } // end namespace
    
} // end namespace

#endif
