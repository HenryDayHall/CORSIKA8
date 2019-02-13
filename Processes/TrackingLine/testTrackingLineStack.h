
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_process_trackinling_teststack_h_
#define _include_process_trackinling_teststack_h_

#include <corsika/geometry/Point.h>
#include <corsika/environment/Environment.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

typedef corsika::units::si::hepmomentum_d MOMENTUM;

struct DummyParticle {
  corsika::units::si::HEPEnergyType fEnergy;
  corsika::geometry::Vector<MOMENTUM> fMomentum;
  corsika::geometry::Point fPosition;
  corsika::environment::BaseNodeType const* fNodePtr;

  DummyParticle(corsika::units::si::HEPEnergyType pEnergy,
                corsika::geometry::Vector<MOMENTUM> pMomentum,
                corsika::geometry::Point pPosition,
                corsika::environment::BaseNodeType const* pNodePtr)
      : fEnergy(pEnergy)
      , fMomentum(pMomentum)
      , fPosition(pPosition)
      , fNodePtr(pNodePtr) {}

  auto GetEnergy() const { return fEnergy; }
  auto GetMomentum() const { return fMomentum; }
  auto GetPosition() const { return fPosition; }
  auto GetPID() const { return corsika::particles::Code::Unknown; }
  auto* GetNode() const { return fNodePtr; }
};

struct DummyStack {
  using ParticleType = DummyParticle;
  using StackIterator = DummyParticle;
};

#endif
