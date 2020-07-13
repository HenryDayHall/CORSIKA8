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

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>
#include <corsika/process/qgsjetII/qgsjet-II-04.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::qgsjetII {

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
  class FragmentsInterface : public corsika::stack::ParticleBase<StackIteratorInterface> {

    using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
    using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

  public:
    void SetParticleData(const int vSize) { SetFragmentSize(vSize); }

    void SetParticleData(FragmentsInterface<StackIteratorInterface>& /*parent*/,
                         const int vSize) {
      SetFragmentSize(vSize);
    }

    void SetFragmentSize(const int v) { GetStackData().SetFragmentSize(GetIndex(), v); }

    double GetFragmentSize() const { return GetStackData().GetFragmentSize(GetIndex()); }
  };

  typedef corsika::stack::Stack<QGSJetIIFragmentsStackData, FragmentsInterface>
      QGSJetIIFragmentsStack;

} // end namespace corsika::process::qgsjetII
