/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/process/QGSJetII/ParticleConversion.hpp>
#include <corsika/process/QGSJetII/qgsjet-II-04.h>
#include <corsika/framework/stack/Stack.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika::qgsjetII {

  typedef corsika::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  class QGSJetIIStackData {

  public:
    void Dump() const {}

    void Clear() {
      qgarr12_.nsp = 0;
      qgarr13_.nsf = 0;
      qgarr55_.nwt = 0;
    }
    unsigned int GetSize() const { return qgarr12_.nsp; }
    unsigned int GetCapacity() const { return nptmax; }

    void SetId(const unsigned int i, const int v) { qgarr14_.ich[i] = v; }
    void SetEnergy(const unsigned int i, const corsika::units::si::HEPEnergyType v) {
      using namespace corsika::units::si;
      qgarr14_.esp[i][0] = v / 1_GeV;
    }

    void SetMomentum(const unsigned int i, const MomentumVector& v) {
      using namespace corsika::units::si;
      auto tmp = v.GetComponents();
      qgarr14_.esp[i][2] = tmp[0] / 1_GeV;
      qgarr14_.esp[i][3] = tmp[1] / 1_GeV;
      qgarr14_.esp[i][1] = tmp[2] / 1_GeV;
    }

    int GetId(const unsigned int i) const { return qgarr14_.ich[i]; }
    corsika::units::si::HEPEnergyType GetEnergy(const int i) const {
      using namespace corsika::units::si;
      return qgarr14_.esp[i][0] * 1_GeV;
    }
    MomentumVector GetMomentum(const unsigned int i,
                               const corsika::CoordinateSystem& CS) const {
      using namespace corsika::units::si;
      geometry::QuantityVector<hepmomentum_d> components = {qgarr14_.esp[i][2] * 1_GeV,
                                                            qgarr14_.esp[i][3] * 1_GeV,
                                                            qgarr14_.esp[i][1] * 1_GeV};
      return MomentumVector(CS, components);
    }

    void Copy(const unsigned int i1, const unsigned int i2) {
      qgarr14_.ich[i2] = qgarr14_.ich[i1];
      for (unsigned int i = 0; i < 4; ++i) qgarr14_.esp[i2][i] = qgarr14_.esp[i1][i];
    }

    void Swap(const unsigned int i1, const unsigned int i2) {
      std::swap(qgarr14_.ich[i1], qgarr14_.ich[i2]);
      for (unsigned int i = 0; i < 4; ++i)
        std::swap(qgarr14_.esp[i1][i], qgarr14_.esp[i2][i]);
    }

    void IncrementSize() { qgarr12_.nsp++; }
    void DecrementSize() {
      if (qgarr12_.nsp > 0) { qgarr12_.nsp--; }
    }
  };

  template <typename StackIteratorInterface>
  class ParticleInterface : public corsika::ParticleBase<StackIteratorInterface> {

    using corsika::ParticleBase<StackIteratorInterface>::GetStackData;
    using corsika::ParticleBase<StackIteratorInterface>::GetIndex;

  public:
    void SetParticleData(const int vID, const corsika::units::si::HEPEnergyType vE,
                         const MomentumVector& vP,
                         const corsika::units::si::HEPMassType) {
      SetPID(vID);
      SetEnergy(vE);
      SetMomentum(vP);
    }

    void SetParticleData(ParticleInterface<StackIteratorInterface>& /*parent*/,
                         const int vID, const corsika::units::si::HEPEnergyType vE,
                         const MomentumVector& vP,
                         const corsika::units::si::HEPMassType) {
      SetPID(vID);
      SetEnergy(vE);
      SetMomentum(vP);
    }

    void SetEnergy(const corsika::units::si::HEPEnergyType v) {
      GetStackData().SetEnergy(GetIndex(), v);
    }

    corsika::units::si::HEPEnergyType GetEnergy() const {
      return GetStackData().GetEnergy(GetIndex());
    }

    void SetPID(const int v) { GetStackData().SetId(GetIndex(), v); }

    corsika::qgsjetII::QgsjetIICode GetPID() const {
      return static_cast<corsika::qgsjetII::QgsjetIICode>(
          GetStackData().GetId(GetIndex()));
    }

    MomentumVector GetMomentum(const corsika::CoordinateSystem& CS) const {
      return GetStackData().GetMomentum(GetIndex(), CS);
    }

    void SetMomentum(const MomentumVector& v) {
      GetStackData().SetMomentum(GetIndex(), v);
    }
  };

  typedef corsika::Stack<QGSJetIIStackData, ParticleInterface> QGSJetIIStack;

} // end namespace corsika::qgsjetII

#endif
