/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_dummystack_h_
#define _include_dummystack_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <string>
#include <vector>

namespace corsika::stack {

  namespace dummy {

    /**
     * Example of a particle object on the stack.
     */

    template <typename _Stack>
    class ParticleRead : public StackIteratorInfo<_Stack, ParticleRead<_Stack> > {

      using StackIteratorInfo<_Stack, ParticleRead>::GetIndex;
      using StackIteratorInfo<_Stack, ParticleRead>::GetStack;

    public:
    };

    /**
     *
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class DummyStackImpl {

    public:
      void Init() {}

      void Clear() {}

      int GetSize() const { return 0; }
      int GetCapacity() const { return 0; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const int i1, const int i2) {}

    protected:
      void IncrementSize() {}
      void DecrementSize() {}

    }; // end class DummyStackImpl

    typedef StackIterator<DummyStackImpl, ParticleRead<DummyStackImpl> > Particle;
    typedef Stack<DummyStackImpl, Particle> DummyStack;

  } // namespace dummy

} // namespace corsika::stack

#endif
