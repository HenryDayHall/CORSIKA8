
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_particleBase_h_
#define _include_particleBase_h_

class StackData; // forward decl

namespace corsika::stack {

  //  template <typename> class PI;// : public ParticleBase<StackIteratorInterface> {
  // template <typename, template <typename> typename> class Stack; // forward decl

  /**
   \class ParticleBase

   The base class to define the readout of particle properties from a
   particle stack. Every stack must implement this readout via the
   ParticleBase class.
  */

  template <typename StackIterator>
  class ParticleBase {

  public:
    ParticleBase() = default;

  private:
    ParticleBase(ParticleBase&) = delete;
    ParticleBase operator=(ParticleBase&) = delete;
    ParticleBase(ParticleBase&&) = delete;
    ParticleBase operator=(ParticleBase&&) = delete;
    ParticleBase(const ParticleBase&) = delete;
    ParticleBase operator=(const ParticleBase&) = delete;

  public:
    /// delete this particle on the stack. The corresponding iterator
    /// will be invalidated by this operation
    void Delete() { GetIterator().GetStack().Delete(GetIterator()); }

    template <typename... Args>
    StackIterator AddSecondary(const Args... v) {
      return GetStack().AddSecondary(GetIterator(), v...);
    }

    //  protected: // todo should be proteced, but don't now how to 'friend Stack'
    /// Function to provide CRTP access to inheriting class (type)
    StackIterator& GetIterator() { return static_cast<StackIterator&>(*this); }
    const StackIterator& GetIterator() const {
      return static_cast<const StackIterator&>(*this);
    }

  protected:
    /// access to underling stack data
    auto& GetStackData() { return GetIterator().GetStackData(); }
    const auto& GetStackData() const { return GetIterator().GetStackData(); }
    auto& GetStack() { return GetIterator().GetStack(); }
    const auto& GetStack() const { return GetIterator().GetStack(); }

    /// return the index number of the underlying iterator object
    int GetIndex() const { return GetIterator().GetIndex(); }
  };

} // namespace corsika::stack

#endif
