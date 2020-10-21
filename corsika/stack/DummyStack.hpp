/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once


#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <string>
#include <tuple>

namespace corsika {


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
    struct ParticleInterface : public corsika::ParticleBase<StackIteratorInterface> {

    	typedef corsika::ParticleBase<StackIteratorInterface> super_type;

    public:
      void SetParticleData(const std::tuple<NoData>& /*v*/) {}
      void SetParticleData(super_type& /*parent*/,
                           const std::tuple<NoData>& /*v*/) {}

      std::string as_string() const { return "dummy-data"; }
    };

    /**
     *
     * Memory implementation of the most simple (no-data) particle stack object.
     */

    class DummyStackImpl {

    public:
    DummyStackImpl()=default;

    DummyStackImpl( DummyStackImpl const&)=default;

    DummyStackImpl(DummyStackImpl &&)=default;

    DummyStackImpl& operator=( DummyStackImpl const& )=default;
    DummyStackImpl& operator=( DummyStackImpl && )=default;


      void init() { entries_ = 0; }

      void clear() { entries_ = 0; }

      int getSize() const { return entries_; }
      int getCapacity() const { return entries_; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void copy(const int /*i1*/, const int /*i2*/) {}

      void incrementSize() { entries_++; }
      void decrementSize() { entries_--; }

	int getEntries() const {
		return entries_;
	}

	void setEntries(int entries = 0) {
		entries_ = entries;
	}

    private:
      int entries_ = 0;

    }; // end class DummyStackImpl

    typedef Stack<DummyStackImpl, ParticleInterface> DummyStack;


} // namespace corsika
