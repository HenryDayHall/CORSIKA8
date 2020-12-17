/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/stack/Stack.hpp>
#include <corsika/modules/sibyll/ParticleConversion.hpp>

#include <sibyll2.3d.hpp>

namespace corsika::sibyll {

  typedef corsika::Vector<hepmomentum_d> MomentumVector;

  class SibStackData {

  public:
    void dump() const {}

    void clear() { s_plist_.np = 0; }
    unsigned int getSize() const { return s_plist_.np; }
    unsigned int getCapacity() const { return 8000; }

    void setId(const unsigned int i, const int v) { s_plist_.llist[i] = v; }
    void setEnergy(const unsigned int i, const HEPEnergyType v) {
      s_plist_.p[3][i] = v / 1_GeV;
    }
    void setMass(const unsigned int i, const HEPMassType v) {
      s_plist_.p[4][i] = v / 1_GeV;
    }
    void setMomentum(const unsigned int i, const MomentumVector& v) {
      auto tmp = v.getComponents();
      for (int idx = 0; idx < 3; ++idx) s_plist_.p[idx][i] = tmp[idx] / 1_GeV;
    }

    int getId(const unsigned int i) const { return s_plist_.llist[i]; }
    HEPEnergyType getEnergy(const int i) const { return s_plist_.p[3][i] * 1_GeV; }
    HEPEnergyType getMass(const unsigned int i) const { return s_plist_.p[4][i] * 1_GeV; }
    MomentumVector getMomentum(const unsigned int i) const {
      CoordinateSystemPtr& rootCS = get_root_CoordinateSystem();
      QuantityVector<hepmomentum_d> components = {
          s_plist_.p[0][i] * 1_GeV, s_plist_.p[1][i] * 1_GeV, s_plist_.p[2][i] * 1_GeV};
      return MomentumVector(rootCS, components);
    }

    void Copy(const unsigned int i1, const unsigned int i2) {
      s_plist_.llist[i2] = s_plist_.llist[i1];
      for (unsigned int i = 0; i < 5; ++i) s_plist_.p[i][i2] = s_plist_.p[i][i1];
    }

    void swap(const unsigned int i1, const unsigned int i2) {
      std::swap(s_plist_.llist[i1], s_plist_.llist[i2]);
      for (unsigned int i = 0; i < 5; ++i)
        std::swap(s_plist_.p[i][i1], s_plist_.p[i][i2]);
    }

    void incrementSize() { s_plist_.np++; }
    void decrementSize() {
      if (s_plist_.np > 0) { s_plist_.np--; }
    }
  };

  template <typename StackIteratorInterface>
  class ParticleInterface : public corsika::ParticleBase<StackIteratorInterface> {

    using corsika::ParticleBase<StackIteratorInterface>::getStackData;
    using corsika::ParticleBase<StackIteratorInterface>::getIndex;

  public:
    void setParticleData(const int vID, // corsika::sibyll::SibyllCode vID,
                         const HEPEnergyType vE, const MomentumVector& vP,
                         const HEPMassType vM) {
      setPID(vID);
      setEnergy(vE);
      setMomentum(vP);
      setMass(vM);
    }

    void setParticleData(ParticleInterface<StackIteratorInterface>& /*parent*/,
                         const int vID, //  corsika::sibyll::SibyllCode vID,
                         const HEPEnergyType vE, const MomentumVector& vP,
                         const HEPMassType vM) {
      setPID(vID);
      setEnergy(vE);
      setMomentum(vP);
      setMass(vM);
    }

    void setEnergy(const HEPEnergyType v) { getStackData().setEnergy(getIndex(), v); }

    HEPEnergyType getEnergy() const { return getStackData().getEnergy(getIndex()); }

    bool hasDecayed() const { return abs(getStackData().getId(getIndex())) > 100; }

    void setMass(const HEPMassType v) { getStackData().setMass(getIndex(), v); }

    HEPEnergyType getMass() const { return getStackData().getMass(getIndex()); }

    void setPID(const int v) { getStackData().setId(getIndex(), v); }

    corsika::sibyll::SibyllCode getPID() const {
      return static_cast<corsika::sibyll::SibyllCode>(getStackData().getId(getIndex()));
    }

    MomentumVector getMomentum() const { return getStackData().getMomentum(getIndex()); }

    void setMomentum(const MomentumVector& v) {
      getStackData().setMomentum(getIndex(), v);
    }
  };

  typedef corsika::Stack<SibStackData, ParticleInterface> SibStack;

} // end namespace corsika::sibyll
