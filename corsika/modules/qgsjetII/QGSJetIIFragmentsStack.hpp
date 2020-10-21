/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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
#include <corsika/modules/qgsjetII/ParticleConversion.hpp>
#include <corsika/modules/qgsjetII/qgsjet-II-04.hpp>

namespace corsika::qgsjetII {

  class QGSJetIIFragmentsStackData {

  public:
    void Init();
    void Dump() const {}

    void Clear() {
      qgarr13_.nsf = 0;
      qgarr55_.nwt = 0;
    }
    unsigned int GetSize() const { return qgarr13_.nsf; }
    unsigned int GetCapacity() const { return iapmax; }

    static unsigned int GetWoundedNucleonsTarget() { return qgarr55_.nwt; }
    static unsigned int GetWoundedNucleonsProjectile() { return qgarr55_.nwp; }

    int GetFragmentSize(const unsigned int i) const { return qgarr13_.iaf[i]; }
    void SetFragmentSize(const unsigned int i, const int v) { qgarr13_.iaf[i] = v; }

    void Copy(const unsigned int i1, const unsigned int i2) {
      qgarr13_.iaf[i2] = qgarr13_.iaf[i1];
    }

    void Swap(const unsigned int i1, const unsigned int i2) {
      std::swap(qgarr13_.iaf[i1], qgarr13_.iaf[i2]);
    }

    void IncrementSize() { qgarr13_.nsf++; }
    void DecrementSize() {
      if (qgarr13_.nsf > 0) { qgarr13_.nsf--; }
    }
  };

  template <typename StackIteratorInterface>
  class FragmentsInterface : public corsika::ParticleBase<StackIteratorInterface> {

    using corsika::ParticleBase<StackIteratorInterface>::GetStackData;
    using corsika::ParticleBase<StackIteratorInterface>::GetIndex;

  public:
    void SetParticleData(const int vSize) { SetFragmentSize(vSize); }

    void SetParticleData(FragmentsInterface<StackIteratorInterface>& /*parent*/,
                         const int vSize) {
      SetFragmentSize(vSize);
    }

    void SetFragmentSize(const int v) { GetStackData().SetFragmentSize(GetIndex(), v); }

    double GetFragmentSize() const { return GetStackData().GetFragmentSize(GetIndex()); }
  };

  typedef corsika::Stack<QGSJetIIFragmentsStackData, FragmentsInterface>
      QGSJetIIFragmentsStack;

} // end namespace corsika::qgsjetII

//#include <corsika/detail/modules/qgsjetII/QGSJetIIFragmentsStack.inl>
