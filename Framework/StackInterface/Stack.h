
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Stack_h__
#define _include_Stack_h__

#include <corsika/stack/StackIteratorInterface.h>

#include <stdexcept>
#include <type_traits>

#include <corsika/stack/SecondaryView.h>

// SFINAE test
template <typename T>
class HasGetIndexFromIterator {
private:
  typedef char YesType[1];
  typedef char NoType[2];

  template <typename C>
  static YesType& test(decltype(&C::GetIndexFromIterator));
  template <typename C>
  static NoType& test(...);

public:
  enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

/**
   All classes around management of particles on a stack.
 */

namespace corsika::stack {

  /**
     This is just a forward declatation for the user-defined
     ParticleInterface, which is one of the essential template
     parameters for the Stack.

     <b>Important:</b> ParticleInterface must inherit from ParticleBase !
   */

  template <typename>
  class ParticleInterface; // forward decl

  /**
     The Stack class provides (and connects) the main particle data storage machinery.

     The StackDataType type is the user-provided bare data storage
     object. This can be of any complexity, from a simple struct
     (fortran common block), to a combination of different and
     distributed data sources.

     The user-provided ParticleInterface template type is the base
     class type of the StackIteratorInterface class (CRTP) and must
     provide all functions to read single particle data from the
     StackDataType, given an 'unsigned int' index.

     The Stack implements the
     std-type begin/end function to allow integration in normal for
     loops, ranges, etc.
   */

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class Stack {

    using StackType = Stack<StackDataType, ParticleInterface>;

    StackDataType fData; ///< this in general holds all the data and can be quite big

  private:
    Stack(Stack&) = delete; ///< since Stack can be very big, we don't want to copy it
    Stack& operator=(Stack&) =
        delete; ///< since Stack can be very big, we don't want to copy it

  public:
    //    template<typename = std::enable_if_t<std::is_reference<StackDataType>{}>>
    Stack(StackDataType vD)
        : fData(vD) {}

    /**
     * This constructor takes any argument and passes it on to the
     * StackDataType user class. If the user did not provide a suited
     * constructor this will fail with an error message.
     */
    template <typename... Args,
              typename = std::enable_if_t<!std::is_reference<StackDataType>{}>>
    Stack(Args... args)
        : fData(args...) {}
    // , typename std::enable_if<!std::is_reference<StackDataType>::value,
    // std::nullptr_t>::type = nullptr)

  public:
    typedef StackDataType
        StackImpl; ///< this is the type of the user-provided data structure

    template <typename SI>
    using PIType = ParticleInterface<SI>;

    /**
     * Via the StackIteratorInterface and ConstStackIteratorInterface
     * specialization, the type of the StackIterator
     * template class is declared for a particular stack data
     * object. Using CRTP, this also determines the type of
     * ParticleInterface template class simultaneously.
     */
    using StackIterator =
        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                               ParticleInterface, StackType>;
    using ConstStackIterator =
        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                    ParticleInterface, StackType>;

    /**
     * this is the full type of the declared ParticleInterface: typedef typename
     */
    typedef typename StackIterator::ParticleInterfaceType ParticleType;

    friend class StackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface,
        StackType>;

    friend class ConstStackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface,
        StackType>;

  public:
    unsigned int GetCapacity() const { return fData.GetCapacity(); }
    unsigned int GetSize() const { return fData.GetSize(); }

    template <typename... Args>
    auto Init(Args... args) {
      return fData.Init(args...);
    }
    template <typename... Args>
    auto Clear(Args... args) {
      return fData.Clear(args...);
    }

  public:
    /**
     * @name These are functions required by std containers and std loops
     * @{
     */
    StackIterator begin() { return StackIterator(*this, 0); }
    StackIterator end() { return StackIterator(*this, GetSize()); }
    StackIterator last() { return StackIterator(*this, GetSize() - 1); }

    ConstStackIterator begin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator end() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator last() const { return ConstStackIterator(*this, GetSize() - 1); }

    ConstStackIterator cbegin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator cend() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator clast() const { return ConstStackIterator(*this, GetSize() - 1); }
    /// @}

    /**
     * increase stack size, create new particle at end of stack
     */
    template <typename... Args>
    StackIterator AddParticle(const Args... v) {
      fData.IncrementSize();
      return StackIterator(*this, GetSize() - 1, v...);
    }

    /**
     * increase stack size, create new particle at end of stack, related to parent
     * particle/projectile
     */
    template <typename... Args>
    StackIterator AddSecondary(StackIterator& parent, const Args... v) {
      fData.IncrementSize();
      return StackIterator(*this, GetSize() - 1, parent, v...);
    }

    void Swap(StackIterator a, StackIterator b) {
      fData.Swap(a.GetIndex(), b.GetIndex());
    }
    void Swap(ConstStackIterator a, ConstStackIterator b) {
      fData.Swap(a.GetIndex(), b.GetIndex());
    }
    void Copy(StackIterator a, StackIterator b) {
      fData.Copy(a.GetIndex(), b.GetIndex());
    }
    void Copy(ConstStackIterator a, StackIterator b) {
      fData.Copy(a.GetIndex(), b.GetIndex());
    }

    /**
     * delete this particle
     */
    void Delete(StackIterator p) {
      if (GetSize() == 0) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (p.GetIndex() < GetSize() - 1) fData.Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
      // p.SetInvalid();
    }
    /**
     * delete this particle
     */
    void Delete(ParticleType p) { Delete(p.GetIterator()); }

    /**
     * delete last particle on stack by decrementing stack size
     */
    void DeleteLast() { fData.DecrementSize(); }

    /**
     * check if there are no further particles on stack
     */
    bool IsEmpty() { return GetSize() == 0; }

    /**
     * return next particle from stack
     */
    StackIterator GetNextParticle() { return last(); }

  protected:
    // typename std::enable_if<HasGetIndexFromIterator<T>::value, unsigned int>::type
    // typename std::enable_if<std::is_base_of<decltype(*this)>,
    // SecondaryView<StackDataType, ParticleInterface>>::value, unsigned int>::type
    unsigned int GetIndexFromIterator(const unsigned int vI) const { return vI; }

    typename std::remove_reference<StackDataType>::type& GetStackData() { return fData; }
    const typename std::remove_reference<StackDataType>::type& GetStackData() const {
      return fData;
    }
  };

} // namespace corsika::stack

#endif
