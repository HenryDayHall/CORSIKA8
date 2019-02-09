#ifndef _include_process_trackinling_teststack_h_
#define _include_process_trackinling_teststack_h_

typedef corsika::units::si::hepmomentum_d MOMENTUM;

struct DummyParticle {
  HEPEnergyType fEnergy;
  Vector<MOMENTUM> fMomentum;
  Point fPosition;

  DummyParticle(HEPEnergyType pEnergy, Vector<MOMENTUM> pMomentum, Point pPosition)
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
