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
#include <corsika/modules/epos/ParticleConversion.hpp>

#include <epos.hpp>

namespace corsika::epos {

  typedef corsika::Vector<hepmomentum_d> MomentumVector;

  class EposStackData {

  public:
    void dump() const {}

    void clear() { ::epos::cptl_.nptl = 0; }
    unsigned int getSize() const { return ::epos::cptl_.nptl; }
    unsigned int getCapacity() const { return ::epos::mxptl; }

    void setId(const unsigned int i, const int v) { ::epos::cptl_.idptl[i] = v; }
    void setEnergy(const unsigned int i, const HEPEnergyType v) {
      ::epos::cptl_.pptl[3][i] = v / 1_GeV;
    }
    void setMass(const unsigned int i, const HEPMassType v) {
      ::epos::cptl_.pptl[4][i] = v / 1_GeV;
    }
    void setMomentum(const unsigned int i, const MomentumVector& v) {
      auto tmp = v.getComponents();
      for (int idx = 0; idx < 3; ++idx) ::epos::cptl_.pptl[idx][i] = tmp[idx] / 1_GeV;
    }
    void setState(const unsigned int i, const int v) { ::epos::cptl_.istptl[i] = v; }

    int getId(const unsigned int i) const { return ::epos::cptl_.idptl[i]; }
    int getState(const unsigned int i) const { return ::epos::cptl_.istptl[i]; }
    HEPEnergyType getEnergy(const int i) const {
      return ::epos::cptl_.pptl[3][i] * 1_GeV;
    }
    HEPEnergyType getMass(const unsigned int i) const {
      return ::epos::cptl_.pptl[4][i] * 1_GeV;
    }
    MomentumVector getMomentum(const unsigned int i) const {
      CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();
      QuantityVector<hepmomentum_d> components = {::epos::cptl_.pptl[0][i] * 1_GeV,
                                                  ::epos::cptl_.pptl[1][i] * 1_GeV,
                                                  ::epos::cptl_.pptl[2][i] * 1_GeV};
      return MomentumVector(rootCS, components);
    }

    MomentumVector getMomentum(const unsigned int i,
                               const CoordinateSystemPtr& CS) const {
      QuantityVector<hepmomentum_d> components = {::epos::cptl_.pptl[0][i] * 1_GeV,
                                                  ::epos::cptl_.pptl[1][i] * 1_GeV,
                                                  ::epos::cptl_.pptl[2][i] * 1_GeV};
      return MomentumVector(CS, components);
    }

    void copy(const unsigned int i1, const unsigned int i2) {
      ::epos::cptl_.idptl[i2] = ::epos::cptl_.idptl[i1];
      ::epos::cptl_.iorptl[i2] = ::epos::cptl_.iorptl[i1];
      ::epos::cptl_.jorptl[i2] = ::epos::cptl_.jorptl[i1];
      ::epos::cptl_.istptl[i2] = ::epos::cptl_.istptl[i1];
      ::epos::cptl_.ityptl[i2] = ::epos::cptl_.ityptl[i1];
      for (unsigned int i = 0; i < 5; ++i)
        ::epos::cptl_.pptl[i][i2] = ::epos::cptl_.pptl[i][i1];
      for (unsigned int i = 0; i < 2; ++i) {
        ::epos::cptl_.tivptl[i][i2] = ::epos::cptl_.tivptl[i][i1];
        ::epos::cptl_.ifrptl[i][i2] = ::epos::cptl_.ifrptl[i][i1];
      }
      for (unsigned int i = 0; i < 4; ++i) {
        ::epos::cptl_.xorptl[i][i2] = ::epos::cptl_.xorptl[i][i1];
        ::epos::cptl_.ibptl[i][i2] = ::epos::cptl_.ibptl[i][i1];
      }
    }

    void swap(const unsigned int i1, const unsigned int i2) {
      std::swap(::epos::cptl_.idptl[i2], ::epos::cptl_.idptl[i1]);
      std::swap(::epos::cptl_.iorptl[i2], ::epos::cptl_.iorptl[i1]);
      std::swap(::epos::cptl_.jorptl[i2], ::epos::cptl_.jorptl[i1]);
      std::swap(::epos::cptl_.istptl[i2], ::epos::cptl_.istptl[i1]);
      std::swap(::epos::cptl_.ityptl[i2], ::epos::cptl_.ityptl[i1]);
      for (unsigned int i = 0; i < 5; ++i)
        std::swap(::epos::cptl_.pptl[i][i2], ::epos::cptl_.pptl[i][i1]);
      for (unsigned int i = 0; i < 2; ++i) {
        std::swap(::epos::cptl_.tivptl[i][i2], ::epos::cptl_.tivptl[i][i1]);
        std::swap(::epos::cptl_.ifrptl[i][i2], ::epos::cptl_.ifrptl[i][i1]);
      }
      for (unsigned int i = 0; i < 4; ++i) {
        std::swap(::epos::cptl_.xorptl[i][i2], ::epos::cptl_.xorptl[i][i1]);
        std::swap(::epos::cptl_.ibptl[i][i2], ::epos::cptl_.ibptl[i][i1]);
      }
    }

    void incrementSize() { ::epos::cptl_.nptl++; }
    void decrementSize() {
      if (::epos::cptl_.nptl > 0) { ::epos::cptl_.nptl--; }
    }
  };

  template <typename StackIteratorInterface>
  class ParticleInterface : public corsika::ParticleBase<StackIteratorInterface> {

    using corsika::ParticleBase<StackIteratorInterface>::getStackData;
    using corsika::ParticleBase<StackIteratorInterface>::getIndex;

  public:
    void setParticleData(const int vID, // corsika::epos::EposCode vID,
                         const HEPEnergyType vE, const MomentumVector& vP,
                         const HEPMassType vM) {
      setPID(vID);
      setEnergy(vE);
      setMomentum(vP);
      setMass(vM);
      setState(0);
    }

    void setParticleData(ParticleInterface<StackIteratorInterface>& /*parent*/,
                         const int vID, //  corsika::epos::EposCode vID,
                         const HEPEnergyType vE, const MomentumVector& vP,
                         const HEPMassType vM) {
      setPID(vID);
      setEnergy(vE);
      setMomentum(vP);
      setMass(vM);
      setState(0);
    }

    void setEnergy(const HEPEnergyType v) { getStackData().setEnergy(getIndex(), v); }

    HEPEnergyType getEnergy() const { return getStackData().getEnergy(getIndex()); }

    bool hasDecayed() const { return getStackData().getState(getIndex()) == 0; }

    void setMass(const HEPMassType v) { getStackData().setMass(getIndex(), v); }

    HEPEnergyType getMass() const { return getStackData().getMass(getIndex()); }

    void setPID(const int v) { getStackData().setId(getIndex(), v); }

    corsika::epos::EposCode getPID() const {
      return static_cast<corsika::epos::EposCode>(getStackData().getId(getIndex()));
    }

    void setState(const int v) { getStackData().setState(getIndex(), v); }

    corsika::epos::EposCode getState() const {
      return static_cast<corsika::epos::EposCode>(getStackData().getState(getIndex()));
    }

    MomentumVector getMomentum() const { return getStackData().getMomentum(getIndex()); }

    MomentumVector getMomentum(const CoordinateSystemPtr& CS) const {
      return getStackData().getMomentum(getIndex(), CS);
    }

    void setMomentum(const MomentumVector& v) {
      getStackData().setMomentum(getIndex(), v);
    }
  };

  typedef corsika::Stack<EposStackData, ParticleInterface> EposStack;

} // end namespace corsika::epos
