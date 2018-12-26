
/**
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

  typedef corsika::geometry::Vector<corsika::units::hep::energy_hep_d> MomentumVector;

  class SibStackData {

  public:
    void Init();

    void Clear() { s_plist_.np = 0; }

    int GetSize() const { return s_plist_.np; }

    int GetCapacity() const { return 8000; }

    void SetId(const int i, const int v) { s_plist_.llist[i] = v; }
    void SetEnergy(const int i, const corsika::units::hep::EnergyType v) {
      using namespace corsika::units::hep;
      s_plist_.p[3][i] = v / 1_GeV;
    }
    void SetMomentum(const int i, const MomentumVector& v) {
      using namespace corsika::units;
      using namespace corsika::units::hep;
      auto tmp = v.GetComponents();
      for (int idx = 0; idx < 3; ++idx) s_plist_.p[idx][i] = tmp[idx] / 1_GeV;
    }

    int GetId(const int i) const { return s_plist_.llist[i]; }

    corsika::units::hep::EnergyType GetEnergy(const int i) const {
      using namespace corsika::units::hep;
      return s_plist_.p[3][i] * 1_GeV;
    }

    MomentumVector GetMomentum(const int i) const {
      using corsika::geometry::CoordinateSystem;
      using corsika::geometry::QuantityVector;
      using corsika::geometry::RootCoordinateSystem;
      using namespace corsika::units::hep;
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
      QuantityVector<energy_hep_d> components = {
          s_plist_.p[0][i] * 1_GeV, s_plist_.p[1][i] * 1_GeV, s_plist_.p[2][i] * 1_GeV};
      MomentumVector v1(rootCS, components);
      return v1;
    }

    void Copy(const int i1, const int i2) {
      s_plist_.llist[i1] = s_plist_.llist[i2];
      s_plist_.p[3][i1] = s_plist_.p[3][i2];
    }

  protected:
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
    void SetEnergy(const corsika::units::hep::EnergyType v) {
      GetStackData().SetEnergy(GetIndex(), v);
    }
    corsika::units::hep::EnergyType GetEnergy() const {
      return GetStackData().GetEnergy(GetIndex());
    }
    bool HasDecayed() const {
      return abs(GetStackData().GetId(GetIndex())) > 100 ? true : false;
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
