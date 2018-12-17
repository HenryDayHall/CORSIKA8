#ifndef _include_sibstack_h_
#define _include_sibstack_h_

#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/stack/super_stupid/SuperStupidStack.h>

using namespace std;
using namespace corsika::stack;
using namespace corsika::units::si;
using namespace corsika::geometry;

namespace corsika::process::sibyll {

  class SibStackData {

  public:
    void Init();

    void Clear() { s_plist_.np = 0; }

    int GetSize() const { return s_plist_.np; }
#warning check actual capacity of sibyll stack
  int GetCapacity() const { return 8000; }

  void SetId(const int i, const int v) { s_plist_.llist[i] = v; }
  void SetEnergy(const int i, const EnergyType v) { s_plist_.p[3][i] = v / 1_GeV; }
  void SetMomentum(const int i, const super_stupid::MomentumVector& v) {
    auto tmp = v.GetComponents();
    for (int idx = 0; idx < 3; ++idx)
      s_plist_.p[idx][i] = tmp[idx] / 1_GeV * constants::c;
  }

  int GetId(const int i) const { return s_plist_.llist[i]; }

  EnergyType GetEnergy(const int i) const { return s_plist_.p[3][i] * 1_GeV; }

  super_stupid::MomentumVector GetMomentum(const int i) const {
    CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
    corsika::geometry::QuantityVector<momentum_d> components{
        s_plist_.p[0][i] * 1_GeV / constants::c,
        s_plist_.p[1][i] * 1_GeV / constants::c,
        s_plist_.p[2][i] * 1_GeV / constants::c};
    super_stupid::MomentumVector v1(rootCS, components);
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
class ParticleInterface : public ParticleBase<StackIteratorInterface> {
  using ParticleBase<StackIteratorInterface>::GetStackData;
  using ParticleBase<StackIteratorInterface>::GetIndex;

public:
  void SetEnergy(const EnergyType v) { GetStackData().SetEnergy(GetIndex(), v); }
  EnergyType GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
  void SetPID(const int v) { GetStackData().SetId(GetIndex(), v); }
  corsika::process::sibyll::SibyllCode GetPID() const {
    return static_cast<corsika::process::sibyll::SibyllCode>(
        GetStackData().GetId(GetIndex()));
  }
  super_stupid::MomentumVector GetMomentum() const {
    return GetStackData().GetMomentum(GetIndex());
  }
  void SetMomentum(const super_stupid::MomentumVector& v) {
    GetStackData().SetMomentum(GetIndex(), v);
  }
};

typedef Stack<SibStackData, ParticleInterface> SibStack;

}
>>>>>>> master

#endif
