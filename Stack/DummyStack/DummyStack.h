/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/logging/Logging.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <tuple>
#include <string>

namespace corsika::stack {

  namespace dummy {

    /**
     * Example of a particle object on the stack, with NO DATA.
     */

    /**
       however, conceptually we need to provide fake data. A stack without data does not
       work...
     */

    struct NoData { /* nothing */
      int nothing = 0;
    };

    template <typename StackIteratorInterface>
    class ParticleInterface
        : public corsika::stack::ParticleBase<StackIteratorInterface> {

    protected:
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetStack;
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;

    public:
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

    public:
      void SetParticleData(const std::tuple<NoData>& /*v*/) {}
      void SetParticleData(ParticleInterface<StackIteratorInterface>& /*parent*/,
                           const std::tuple<NoData>& /*v*/) {}

      std::string as_string() const { return "dummy-data"; }
    };

    /**
     *
     * Memory implementation of the most simple (no-data) particle stack object.
     */

    class DummyStackImpl {

    public:
      void Init() { entries_ = 0; }

      void Clear() { entries_ = 0; }

      int GetSize() const { return entries_; }
      int GetCapacity() const { return entries_; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const int /*i1*/, const int /*i2*/) {}

      void IncrementSize() { entries_++; }
      void DecrementSize() { entries_--; }

    private:
      int entries_ = 0;

    }; // end class DummyStackImpl

    typedef Stack<DummyStackImpl, ParticleInterface> DummyStack;

  } // namespace dummy

} // namespace corsika::stack
