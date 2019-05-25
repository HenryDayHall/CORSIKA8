/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_sibstack_h_
#define _include_sibstack_h_

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::sibyll {

  typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  class SibStackData {

  public:
    void Init();
    void Dump() const {}

    void Clear() { s_plist_.np = 0; }
    unsigned int GetSize() const { return s_plist_.np; }
    unsigned int GetCapacity() const { return 8000; }

    void SetId(const unsigned int i, const int v) { s_plist_.llist[i] = v; }
    void SetEnergy(const unsigned int i, const corsika::units::si::HEPEnergyType v) {
      using namespace corsika::units::si;
      s_plist_.p[3][i] = v / 1_GeV;
    }
    void SetMass(const unsigned int i, const corsika::units::si::HEPMassType v) {
      using namespace corsika::units::si;
      s_plist_.p[4][i] = v / 1_GeV;
    }
    void SetMomentum(const unsigned int i, const MomentumVector& v) {
      using namespace corsika::units::si;
      auto tmp = v.GetComponents();
      for (int idx = 0; idx < 3; ++idx) s_plist_.p[idx][i] = tmp[idx] / 1_GeV;
    }

    int GetId(const unsigned int i) const { return s_plist_.llist[i]; }
    corsika::units::si::HEPEnergyType GetEnergy(const int i) const {
      using namespace corsika::units::si;
      return s_plist_.p[3][i] * 1_GeV;
    }
    corsika::units::si::HEPEnergyType GetMass(const unsigned int i) const {
      using namespace corsika::units::si;
      return s_plist_.p[4][i] * 1_GeV;
    }
    MomentumVector GetMomentum(const unsigned int i) const {
      using corsika::geometry::CoordinateSystem;
      using corsika::geometry::QuantityVector;
      using corsika::geometry::RootCoordinateSystem;
      using namespace corsika::units::si;
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
      QuantityVector<hepmomentum_d> components = {
          s_plist_.p[0][i] * 1_GeV, s_plist_.p[1][i] * 1_GeV, s_plist_.p[2][i] * 1_GeV};
      return MomentumVector(rootCS, components);
    }

    void Copy(const unsigned int i1, const unsigned int i2) {
      s_plist_.llist[i2] = s_plist_.llist[i1];
      for (unsigned int i = 0; i < 5; ++i) s_plist_.p[i][i2] = s_plist_.p[i][i1];
    }

    void Swap(const unsigned int i1, const unsigned int i2) {
      std::swap(s_plist_.llist[i1], s_plist_.llist[i2]);
      for (unsigned int i = 0; i < 5; ++i)
        std::swap(s_plist_.p[i][i1], s_plist_.p[i][i2]);
    }

    void IncrementSize() { s_plist_.np++; }
    void DecrementSize() {
      if (s_plist_.np > 0) { s_plist_.np--; }
    }
  };

  template <typename StackIteratorInterface>
  class ParticleInterface : public corsika::stack::ParticleBase<StackIteratorInterface> {

    using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
    using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

  public:
    void SetParticleData(const int vID, // corsika::process::sibyll::SibyllCode vID,
                         const corsika::units::si::HEPEnergyType vE,
                         const MomentumVector& vP,
                         const corsika::units::si::HEPMassType vM) {
      SetPID(vID);
      SetEnergy(vE);
      SetMomentum(vP);
      SetMass(vM);
    }

    void SetParticleData(ParticleInterface<StackIteratorInterface>& /*parent*/,
                         const int vID, //  corsika::process::sibyll::SibyllCode vID,
                         const corsika::units::si::HEPEnergyType vE,
                         const MomentumVector& vP,
                         const corsika::units::si::HEPMassType vM) {
      SetPID(vID);
      SetEnergy(vE);
      SetMomentum(vP);
      SetMass(vM);
    }

    void SetEnergy(const corsika::units::si::HEPEnergyType v) {
      GetStackData().SetEnergy(GetIndex(), v);
    }

    corsika::units::si::HEPEnergyType GetEnergy() const {
      return GetStackData().GetEnergy(GetIndex());
    }

    bool HasDecayed() const { return abs(GetStackData().GetId(GetIndex())) > 100; }

    void SetMass(const corsika::units::si::HEPMassType v) {
      GetStackData().SetMass(GetIndex(), v);
    }

    corsika::units::si::HEPEnergyType GetMass() const {
      return GetStackData().GetMass(GetIndex());
    }

    void SetPID(const int v) { GetStackData().SetId(GetIndex(), v); }

    corsika::process::sibyll::SibyllCode GetPID() const {
      return static_cast<corsika::process::sibyll::SibyllCode>(
          GetStackData().GetId(GetIndex()));
    }

    MomentumVector GetMomentum() const { return GetStackData().GetMomentum(GetIndex()); }

    void SetMomentum(const MomentumVector& v) {
      GetStackData().SetMomentum(GetIndex(), v);
    }
  };

  typedef corsika::stack::Stack<SibStackData, ParticleInterface> SibStack;

} // end namespace corsika::process::sibyll

#endif
