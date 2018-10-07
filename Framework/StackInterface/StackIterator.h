
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_StackIterator_h__
#define _include_StackIterator_h__

#include <corsika/stack/ParticleBase.h>

#include <iomanip>
#include <iostream>

class StackData; // forward decl

namespace corsika::stack {

  template <typename StackData, template <typename> typename ParticleInterface>
  class Stack; // forward decl

  /**
     @class StackIterator

     The StackIterator is the main interface to iterator over
     particles on a stack. At the same time StackIterator is a
     Particle object by itself, thus there is no difference between
     type and ref_type for convenience of the physicist.

     This allows to write code like
     \verbatim
     for (auto& p : theStack) { p.SetEnergy(newEnergy); }
     \endverbatim

     The template argument Stack determines the type of Stack object
     the data is stored in. A pointer to the Stack object is part of
     the StackIterator. In addition to Stack the iterator only knows
     the index fIndex in the Stack data.

     The template argument Particles acts as a policy to provide
     readout function of Particle data from the stack. The Particle
     class must know how to retrieve information from the Stack data
     for a particle entry at any index fIndex.
  */

  template <typename StackData, template <typename> typename ParticleInterface>
  class StackIteratorInterface
      : public ParticleInterface<StackIteratorInterface<StackData, ParticleInterface> > {

    typedef Stack<StackData, ParticleInterface> StackType;
    typedef ParticleInterface<StackIteratorInterface<StackData, ParticleInterface> >
        ParticleInterfaceType;

    // friend class ParticleInterface<StackIterator<StackData>>; // to access GetStackData
    friend class Stack<StackData, ParticleInterface>;  // for access to GetIndex
    friend class ParticleBase<StackIteratorInterface>; // for access to GetStackData

  private:
    int fIndex = 0;
    StackType* fData = 0; // todo is this problematic, when stacks are copied?

  public:
    // StackIterator() : fData(0), fIndex(0) { }
    StackIteratorInterface(StackType& data, const int index)
        : fData(&data)
        , fIndex(index) {}

  private:
    StackIteratorInterface(const StackIteratorInterface& mit)
        : fData(mit.fData)
        , fIndex(mit.fIndex) {}

  public:
    StackIteratorInterface& operator=(const StackIteratorInterface& mit) {
      fData = mit.fData;
      fIndex = mit.fIndex;
      return *this;
    }

  public:
    StackIteratorInterface& operator++() {
      ++fIndex;
      return *this;
    }
    StackIteratorInterface operator++(int) {
      StackIteratorInterface tmp(*this);
      ++fIndex;
      return tmp;
    }
    bool operator==(const StackIteratorInterface& rhs) { return fIndex == rhs.fIndex; }
    bool operator!=(const StackIteratorInterface& rhs) { return fIndex != rhs.fIndex; }

    ParticleInterfaceType& operator*() {
      return static_cast<ParticleInterfaceType&>(*this);
    }
    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }

  protected:
    int GetIndex() const { return fIndex; }
    StackType& GetStack() { return *fData; }
    const StackType& GetStack() const { return *fData; }
    StackData& GetStackData() { return fData->GetStackData(); }
    const StackData& GetStackData() const { return fData->GetStackData(); }

  }; // end class StackIterator

} // namespace corsika::stack

#endif
