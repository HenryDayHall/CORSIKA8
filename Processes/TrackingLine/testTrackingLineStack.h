#ifndef _include_process_trackinling_teststack_h_
#define _include_process_trackinling_teststack_h_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

typedef corsika::units::si::hepmomentum_d MOMENTUM;

struct DummyParticle {
  corsika::units::si::HEPEnergyType fEnergy;
  corsika::geometry::Vector<MOMENTUM> fMomentum;
  corsika::geometry::Point fPosition;

  DummyParticle(corsika::units::si::HEPEnergyType pEnergy,
                corsika::geometry::Vector<MOMENTUM> pMomentum,
                corsika::geometry::Point pPosition)
      : fEnergy(pEnergy)
      , fMomentum(pMomentum)
      , fPosition(pPosition) {}

  auto GetEnergy() const { return fEnergy; }
  auto GetMomentum() const { return fMomentum; }
  auto GetPosition() const { return fPosition; }
  auto GetPID() const { return corsika::particles::Code::Unknown; }
};

struct DummyStack {
  using ParticleType = DummyParticle;
};

#endif
