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
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/stack/SuperStupidStack.hpp>

#include <corsika/logging/Logging.h>

#include <algorithm>
#include <tuple>
#include <vector>

namespace corsika {

/**
 * @namespace nuclear_extension
 *
 * Add A and Z data to existing stack (currently SuperStupidStack) of particle
 * properties. This is done via inheritance, not via CombinedStack since the nuclear
 * data is stored ONLY when needed (for nuclei) and not for all particles. Thus, this is
 * a new, derived Stack object.
 *
 * Only for Code::Nucleus particles A and Z are stored, not for all
 * normal elementary particles.
 *
 * Thus in your code, make sure to always check <code>
 * particle.GetPID()==Code::Nucleus </code> before attempting to
 * read any nuclear information.
 *
 *
 */


/**
 * @class NuclearParticleInterface
 *
 * Define ParticleInterface for NuclearStackExtension Stack derived from
 * ParticleInterface of Inner stack class
 */
template < template <typename> class InnerParticleInterface, typename StackIteratorInterface>
struct NuclearParticleInterface : public  InnerParticleInterface<StackIteratorInterface>  {

	typedef  InnerParticleInterface<StackIteratorInterface>  super_type;



public:


	typedef std::tuple<
			corsika::Code, corsika::units::si::HEPEnergyType,
			momentum_type, corsika::Point,
		    corsika::units::si::TimeType> particle_data_type;

	typedef std::tuple<
			corsika::Code, corsika::units::si::HEPEnergyType,
			momentum_type, corsika::Point,
			corsika::units::si::TimeType,
			unsigned short, unsigned short> altenative_particle_data_type;


	typedef corsika::Vector<corsika::units::si::hepmomentum_d> momentum_type;

	void setParticleData(particle_data_type const& v) {

		if (std::get<0>(v) == corsika::Code::Nucleus) {
			std::ostringstream err;
			err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
			throw std::runtime_error(err.str());
		}

		super_type::setParticleData(v);
		setNucleusRef(-1); // this is not a nucleus
	}

	void setParticleData( altenative_particle_data_type const& v)
	{
		const unsigned short A = std::get<5>(v);
		const unsigned short Z = std::get<6>(v);
		if (std::get<0>(v) != corsika::Code::Nucleus || A == 0 || Z == 0) {
			std::ostringstream err;
			err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
			throw std::runtime_error(err.str());
		}
		setNucleusRef(super_type::GetStackData().getNucleusNextRef()); // store this nucleus data ref
		setNuclearA(A);
		setNuclearZ(Z);
		super_type::setParticleData(particle_data_type{std::get<0>(v), std::get<1>(v),
			std::get<2>(v), std::get<3>(v),	std::get<4>(v)});
	}

	void setParticleData( super_type& p, particle_data_type const& v)
	{
		if (std::get<0>(v) == corsika::Code::Nucleus) {
			std::ostringstream err;
			err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
			throw std::runtime_error(err.str());
		}

		super_type::setParticleData(p, particle_data_type{std::get<0>(v), std::get<1>(v),
			std::get<2>(v), std::get<3>(v),	std::get<4>(v)});

		setNucleusRef(-1); // this is not a nucleus
	}

	void setParticleData( super_type& p, altenative_particle_data_type const& v) {

		const unsigned short A = std::get<5>(v);
		const unsigned short Z = std::get<6>(v);

		if (std::get<0>(v) != corsika::Code::Nucleus || A == 0 || Z == 0) {
			std::ostringstream err;
			err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
			throw std::runtime_error(err.str());
		}

		setNucleusRef(super_type::GetStackData().getNucleusNextRef()); // store this nucleus data ref
		setNuclearA(A);
		setNuclearZ(Z);
		super_type::setParticleData(p, particle_data_type{std::get<0>(v), std::get<1>(v),
			std::get<2>(v), std::get<3>(v),
			std::get<4>(v)});
	}

	std::string as_string() const {
		return fmt::format(
				"{}, nuc({})", super_type::as_string(),
				(isNucleus() ? fmt::format("A={}, Z={}", getNuclearA(), getNuclearZ())
						: "n/a"));
	}

	/**
	 * @name individual setters
	 * @{
	 */
	void setNuclearA(const unsigned short vA) {
		super_type::GetStackData().setNuclearA(super_type::GetIndex(), vA);
	}
	void setNuclearZ(const unsigned short vZ) {
		super_type::GetStackData().setNuclearZ(super_type::GetIndex(), vZ);
	}
	/// @}

	/**
	 * @name individual getters
	 * @{
	 */
	int getNuclearA() const { return super_type::GetStackData().getNuclearA(super_type::GetIndex()); }
	int getNuclearZ() const { return super_type::GetStackData().getNuclearZ(super_type::GetIndex()); }
	/// @}

	/**
	 * Overwrite normal GetParticleMass function with nuclear version
	 */
	corsika::units::si::HEPMassType getMass() const {
		if (super_type::GetPID() ==
				corsika::Code::Nucleus)
			return corsika::GetNucleusMass(getNuclearA(), getNuclearZ());
		return super_type::getMass();
	}
	/**
	 * Overwirte normal GetChargeNumber function with nuclear version
	 **/
	int16_t getChargeNumber() const {
		if (super_type::GetPID() ==
				corsika::Code::Nucleus)
			return getNuclearZ();
		return super_type::getChargeNumber();
	}

	int getNucleusRef() const {
		return super_type::GetStackData().getNucleusRef(GetIndex());
	} // LCOV_EXCL_LINE

protected:

	void setNucleusRef(const int vR) {
		super_type::GetStackData().setNucleusRef(super_type::GetIndex(), vR);
	}

	bool isNucleus() const {
		return super_type::GetStackData().isNucleus(super_type::GetIndex());
	}
};

/**
 * @class NuclearStackExtension
 *
 * Memory implementation of adding nuclear inforamtion to the
 * existing particle stack defined in class InnerStackImpl.
 *
 * Inside the NuclearStackExtension class there is a dedicated
 * fNucleusRef index, where fNucleusRef[i] is referring to the
 * correct A and Z for a specific particle index i. fNucleusRef[i]
 * == -1 means that this is not a nucleus, and a subsequent call to
 * GetNucleusA would produce an exception.
 */
template <typename InnerStackImpl>
class NuclearStackExtensionImpl : public InnerStackImpl {

	typedef InnerStackImpl super_type;

public:

	typedef std::vector<int>            nucleus_ref_type;
	typedef std::vector<unsigned short>   nuclear_a_type;
	typedef std::vector<unsigned short>   nuclear_z_type;


	NuclearStackExtensionImpl()= default;

	NuclearStackExtensionImpl( NuclearStackExtensionImpl<InnerStackImpl> const&)= default;

	NuclearStackExtensionImpl( NuclearStackExtensionImpl<InnerStackImpl> &&)= default;

	NuclearStackExtensionImpl<InnerStackImpl>&
	operator=( NuclearStackExtensionImpl<InnerStackImpl> const&)= default;

	NuclearStackExtensionImpl<InnerStackImpl>&
	operator=( NuclearStackExtensionImpl<InnerStackImpl> &&)= default;

	void init() {
		super_type::init();
	}

	void dump() {
		super_type::dump();
	}

	void clear() {
		super_type::clear();
		nucleusRef_.clear();
		nuclearA_.clear();
		nuclearZ_.clear();
	}

	unsigned int getSize() const {
		return nucleusRef_.size();
	}

	unsigned int getCapacity() const {
		return nucleusRef_.capacity();
	}

	void setNuclearA(const unsigned int i, const unsigned short vA) {
		nuclearA_[getNucleusRef(i)] = vA;
	}

	void setNuclearZ(const unsigned int i, const unsigned short vZ) {
		nuclearZ_[getNucleusRef(i)] = vZ;
	}

	void setNucleusRef(const unsigned int i, const int v) {
		nucleusRef_[i] = v;
	}

	int getNuclearA(const unsigned int i) const {
		return nuclearA_[getNucleusRef(i)];
	}

	int getNuclearZ(const unsigned int i) const {
		return nuclearZ_[getNucleusRef(i)];
	}
	// this function will create new storage for Nuclear Properties, and return the
			// reference to it
	int getNucleusNextRef() {
		nuclearA_.push_back(0);
		nuclearZ_.push_back(0);
		return nuclearA_.size() - 1;
	}

	int getNucleusRef(const unsigned int i) const {
		if (nucleusRef_[i] >= 0) return nucleusRef_[i];
		std::ostringstream err;
		err << "NuclearStackExtension: no nucleus at ref=" << i;
		throw std::runtime_error(err.str());
	}

	bool isNucleus(const unsigned int i) const { return nucleusRef_[i] >= 0; }

	/**
	 *   Function to copy particle at location i1 in stack to i2
	 */
	void copy(const unsigned int i1, const unsigned int i2) {
		// index range check
		if (i1 >= getSize() || i2 >= getSize()) {
			std::ostringstream err;
			err << "NuclearStackExtension: trying to access data beyond size of stack!";
			throw std::runtime_error(err.str());
		}
		// copy internal particle data p[i2] = p[i1]
		super_type::copy(i1, i2);
		// check if any of p[i1] or p[i2] was a Code::Nucleus
		const int ref1 = nucleusRef_[i1];
		const int ref2 = nucleusRef_[i2];
		if (ref2 < 0) {
			if (ref1 >= 0) {
				// i1 is nucleus, i2 is not
				nucleusRef_[i2] = getNucleusNextRef();
				nuclearA_[nucleusRef_[i2]] = nuclearA_[ref1];
				nuclearZ_[nucleusRef_[i2]] = nuclearZ_[ref1];
			} else {
				// neither i1 nor i2 are nuclei
			}
		} else {
			if (ref1 >= 0) {
				// both are nuclei, i2 is overwritten with nucleus i1
				// fNucleusRef stays the same, but A and Z data is overwritten
				nuclearA_[ref2] = nuclearA_[ref1];
				nuclearZ_[ref2] = nuclearZ_[ref1];
			} else {
				// i2 is overwritten with non-nucleus i1
				nucleusRef_[i2] = -1;                       // flag as non-nucleus
				nuclearA_.erase(nuclearA_.cbegin() + ref2); // remove data for i2
				nuclearZ_.erase(nuclearZ_.cbegin() + ref2); // remove data for i2
				const int n = nucleusRef_.size(); // update fNucleusRef: indices above ref2
				// must be decremented by 1
				for (int i = 0; i < n; ++i) {
					if (nucleusRef_[i] > ref2) { nucleusRef_[i] -= 1; }
				}
			}
		}
	}

	/**
	 *   Function to copy particle at location i2 in stack to i1
	 */
	void swap(const unsigned int i1, const unsigned int i2) {
		// index range check
		if (i1 >= getSize() || i2 >= getSize()) {
			std::ostringstream err;
			err << "NuclearStackExtension: trying to access data beyond size of stack!";
			throw std::runtime_error(err.str());
		}
		// swap original particle data
		super_type::swap(i1, i2);
		// swap corresponding nuclear reference data
		std::swap(nucleusRef_[i2], nucleusRef_[i1]);
	}

	void incrementSize() {
		super_type::incrementSize();
		nucleusRef_.push_back(-1);
	}

	void decrementSize() {
		super_type::decrementSize();
		if (nucleusRef_.size() > 0) {
			const int ref = nucleusRef_.back();
			nucleusRef_.pop_back();
			if (ref >= 0) {
				nuclearA_.erase(nuclearA_.begin() + ref);
				nuclearZ_.erase(nuclearZ_.begin() + ref);
				const int n = nucleusRef_.size();
				for (int i = 0; i < n; ++i) {
					if (nucleusRef_[i] >= ref) { nucleusRef_[i] -= 1; }
				}
			}
		}
	}

private:
	/// the actual memory to store particle data

	nucleus_ref_type nucleusRef_;
	nuclear_a_type     nuclearA_;
	nuclear_z_type     nuclearZ_;

}; // end class NuclearStackExtensionImpl

template <typename InnerStack, template <typename> typename _PI>
using NuclearStackExtension =
		Stack<NuclearStackExtensionImpl<typename InnerStack::StackImpl>, _PI> ;

//
template <typename StackIter>
using ExtendedParticleInterfaceType = NuclearParticleInterface<SuperStupidStack, StackIter>;

// the particle data stack with extra nuclear information:
using ParticleDataStack = NuclearStackExtension<SuperStupidStack, ExtendedParticleInterfaceType>;


} // namespace corsika
