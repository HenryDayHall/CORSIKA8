
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

#include <type_traits>

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
      : public ParticleInterface<StackIteratorInterface<
            StackData /*typename std::decay<StackData>::type*/, ParticleInterface>> {

    typedef Stack<StackData, ParticleInterface> StackType;
    /*typedef
        typename std::conditional<std::is_const<StackData>::value,
                                  const Stack<const StackData, ParticleInterface>&,
                                  Stack<StackData, ParticleInterface>&>::type StackType;*/
    typedef ParticleInterface<StackIteratorInterface<StackData, ParticleInterface>>
        ParticleInterfaceType;

    // friend class ParticleInterface<StackIterator<StackData>>; // to access GetStackData
    friend class Stack<StackData, ParticleInterface>;  // for access to GetIndex
    friend class ParticleBase<StackIteratorInterface>; // for access to GetStackData

  private:
    int fIndex = 0;
    StackType* fData = 0; // info: Particles and StackIterators become invalid when parent
                          // Stack is copied or deleted!

    StackIteratorInterface() = delete;

  public:
    /*
    StackIteratorInterface(const StackType& data, const int index)
      : fIndex(index)
      , fData(&data) {}*/
    StackIteratorInterface(StackType& data, const int index)
        : fIndex(index)
        , fData(&data) {}
    /*
  StackIteratorInterface(StackType& data, const int index, typename
  std::enable_if<!std::is_const<StackData>::value>::type* = 0) : fIndex(index) ,
  fData(&data) {}

  StackIteratorInterface(const StackType& data, const int index, typename
  std::enable_if<std::is_const<StackData>::value>::type* = 0) : fIndex(index) ,
  fData(&data) {}
    */
    template <typename... Args>
    StackIteratorInterface(StackType& data, const int index, const Args... args)
        : fIndex(index)
        , fData(&data) {
      (**this).SetParticleData(args...);
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
    StackData& /*typename std::decay<StackData>::type&*/ GetStackData() {
      return fData->GetStackData();
    }
    const StackData& /*typename std::decay<StackData>::type&*/ GetStackData() const {
      return fData->GetStackData();
    }
  }; // end class StackIterator

  template <typename StackData, template <typename> typename ParticleInterface>
  class ConstStackIteratorInterface
      : public ParticleInterface<ConstStackIteratorInterface<
            StackData /*typename std::decay<StackData>::type*/, ParticleInterface>> {

    typedef Stack<StackData, ParticleInterface> StackType;
    /*typedef
        typename std::conditional<std::is_const<StackData>::value,
                                  const Stack<const StackData, ParticleInterface>&,
                                  Stack<StackData, ParticleInterface>&>::type StackType;*/
    typedef ParticleInterface<ConstStackIteratorInterface<StackData, ParticleInterface>>
        ParticleInterfaceType;

    // friend class ParticleInterface<StackIterator<StackData>>; // to access GetStackData
    friend class Stack<StackData, ParticleInterface>;       // for access to GetIndex
    friend class ParticleBase<ConstStackIteratorInterface>; // for access to GetStackData

  private:
    int fIndex = 0;
    const StackType* fData = 0; // info: Particles and StackIterators become invalid when
                                // parent Stack is copied or deleted!

    ConstStackIteratorInterface() = delete;

  public:
    // StackIteratorInterface(const StackType& data, const int index)
    //     : fIndex(index)
    //  , fData(&data) {}
    ConstStackIteratorInterface(const StackType& data, const int index)
        : fIndex(index)
        , fData(&data) {}
    /*
  StackIteratorInterface(StackType& data, const int index, typename
  std::enable_if<!std::is_const<StackData>::value>::type* = 0) : fIndex(index) ,
  fData(&data) {}

  StackIteratorInterface(const StackType& data, const int index, typename
  std::enable_if<std::is_const<StackData>::value>::type* = 0) : fIndex(index) ,
  fData(&data) {}
    */
    template <typename... Args>
    ConstStackIteratorInterface(StackType data, const int index, const Args... args)
        : fIndex(index)
        , fData(data) {
      (**this).SetParticleData(args...);
    }

  public:
    ConstStackIteratorInterface& operator++() {
      ++fIndex;
      return *this;
    }
    ConstStackIteratorInterface operator++(int) {
      ConstStackIteratorInterface tmp(*this);
      ++fIndex;
      return tmp;
    }
    bool operator==(const ConstStackIteratorInterface& rhs) {
      return fIndex == rhs.fIndex;
    }
    bool operator!=(const ConstStackIteratorInterface& rhs) {
      return fIndex != rhs.fIndex;
    }

    ParticleInterfaceType& operator*() {
      return static_cast<ParticleInterfaceType&>(*this);
    }
    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }

  protected:
    int GetIndex() const { return fIndex; }
    // StackType GetStack() { return *fData; }
    const StackType& GetStack() const { return *fData; }
    // StackData& /*typename std::decay<StackData>::type&*/ GetStackData() { return
    // fData->GetStackData(); }
    const StackData& /*typename std::decay<StackData>::type&*/ GetStackData() const {
      return fData->GetStackData();
    }
  }; // end class ConstStackIterator

} // namespace corsika::stack

#endif
