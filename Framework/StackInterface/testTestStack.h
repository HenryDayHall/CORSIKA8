/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_stack_testTestStack_h_
#define _include_corsika_stack_testTestStack_h_

#include <corsika/stack/Stack.h>
#include <tuple>
#include <vector>

/**
 * definition of stack-data object for unit tests
 *
 * TestStackData contain only a single variable "Data" stored in fData
 * with Get/SetData functions.
 */
class TestStackData {

public:
  // these functions are needed for the Stack interface
  void Init() {}
  void Clear() { fData.clear(); }
  unsigned int GetSize() const { return fData.size(); }
  unsigned int GetCapacity() const { return fData.size(); }
  void Copy(const unsigned int i1, const unsigned int i2) { fData[i2] = fData[i1]; }
  void Swap(const unsigned int i1, const unsigned int i2) {
    double tmp0 = fData[i1];
    fData[i1] = fData[i2];
    fData[i2] = tmp0;
  }

  // custom data access function
  void SetData(const unsigned int i, const double v) { fData[i] = v; }
  double GetData(const unsigned int i) const { return fData[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fData.push_back(0.); }
  void DecrementSize() {
    if (fData.size() > 0) { fData.pop_back(); }
  }

  // custom private data section
private:
  std::vector<double> fData;
};

/**
 * From static_cast of a StackIterator over entries in the
 * TestStackData class you get and object of type
 * TestParticleInterface defined here
 *
 * It provides Get/Set methods to read and write data to the "Data"
 * storage of TestStackData obtained via
 * "StackIteratorInterface::GetStackData()", given the index of the
 * iterator "StackIteratorInterface::GetIndex()"
 *
 */
template <typename StackIteratorInterface>
class TestParticleInterface
    : public corsika::stack::ParticleBase<StackIteratorInterface> {

public:
  using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
  using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

  /*
     The SetParticleData methods are called for creating new entries
     on the stack. You can specifiy various parametric versions to
     perform this task:
  */

  // default version for particle-creation from input data
  void SetParticleData(const std::tuple<double> v) { SetData(std::get<0>(v)); }
  void SetParticleData(TestParticleInterface<StackIteratorInterface>& /*parent*/,
                       std::tuple<double> v) {
    SetData(std::get<0>(v));
  }
  /// alternative set-particle data for non-standard construction from different inputs
  /*
  void SetParticleData(const double v, const double p) { SetData(v + p); }
  void SetParticleData(TestParticleInterface<StackIteratorInterface>&,
                       const double v, const double p) {
    SetData(v + p);
    }*/

  // here are the fundamental methods for access to TestStackData data
  void SetData(const double v) { GetStackData().SetData(GetIndex(), v); }
  double GetData() const { return GetStackData().GetData(GetIndex()); }
};

#endif
