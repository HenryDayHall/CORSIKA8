#ifndef PARTICLEPROPERTIES_HPP
#define PARTICLEPROPERTIES_HPP
#include <array>
#include <cstdint>
#include <iostream>

namespace ParticleProperties {

typedef int16_t PDGCode;

#include "generated_particle_properties.inc"

auto constexpr getMass(InternalParticleCode const p)
{
    return masses[static_cast<uint8_t const>(p)];
}

auto constexpr getPDG(InternalParticleCode const p)
{
    return pdg_codes[static_cast<uint8_t const>(p)];
}

auto constexpr getElectricChargeQN(InternalParticleCode const p)
{
    return electric_charge[static_cast<uint8_t const>(p)];
}

auto constexpr getElectricCharge(InternalParticleCode const p)
{
    return getElectricChargeQN(p) * (phys::units::e / 3.);
}

auto const getName(InternalParticleCode const p)
{
    return names[static_cast<uint8_t const>(p)];
}

std::ostream& operator<< (std::ostream& stream, InternalParticleCode const p)
{
    stream << getName(p);
    return stream;
}

}
#endif
