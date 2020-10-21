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
#include <corsika/framework/geometry/RootCoordinateSystem.hpp> // remove
#include <corsika/framework/geometry/Vector.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace corsika {


/**
 * Example of a particle object on the stack.
 */

template <typename StackIteratorInterface>
struct ParticleInterface : public ParticleBase<StackIteratorInterface> {

private:

	typedef corsika::ParticleBase<StackIteratorInterface> super_type;

public:

	typedef corsika::Vector<corsika::units::si::hepmomentum_d> momentum_vector_type;

	std::string as_string() const {
		using namespace corsika::units::si;
		return fmt::format("particle: i={}, PID={}, E={}GeV", super_type::GetIndex(),
				particles::GetName(this->getPID()), this->getEnergy() / 1_GeV);
	}

	void setParticleData( std::tuple<corsika::Code, corsika::units::si::HEPEnergyType,
			momentum_vector_type, corsika::Point, corsika::units::si::TimeType> const& v) {
		this->setPID(std::get<0>(v));
		this->setEnergy(std::get<1>(v));
		this->setMomentum(std::get<2>(v));
		this->setPosition(std::get<3>(v));
		this->setTime(std::get<4>(v));
	}

	void setParticleData( ParticleInterface<StackIteratorInterface> const&,
			std::tuple<corsika::Code, corsika::units::si::HEPEnergyType,
			momentum_vector_type, corsika::Point, corsika::units::si::TimeType> const& v) {
		this->setPID(std::get<0>(v));
		this->setEnergy(std::get<1>(v));
		this->setMomentum(std::get<2>(v));
		this->setPosition(std::get<3>(v));
		this->setTime(std::get<4>(v));
	}

	/// individual setters
	void setPID(const corsika::Code id) {
		super_type::GetStackData().setPID(super_type::GetIndex(), id);
	}
	void setEnergy(const corsika::units::si::HEPEnergyType& e) {
		super_type::GetStackData().setEnergy(super_type::GetIndex(), e);
	}
	void setMomentum(const momentum_vector_type& v) {
		super_type::GetStackData().setMomentum(super_type::GetIndex(), v);
	}
	void setPosition(const corsika::Point& v) {
		super_type::GetStackData().setPosition(super_type::GetIndex(), v);
	}
	void setTime(const corsika::units::si::TimeType& v) {
		super_type::GetStackData().setTime(super_type::GetIndex(), v);
	}

	/// individual getters
	corsika::Code getPID() const {
		return super_type::GetStackData().getPID(super_type::GetIndex());
	}
	corsika::units::si::HEPEnergyType getEnergy() const {
		return super_type::GetStackData().getEnergy(super_type::GetIndex());
	}
	momentum_vector_type getMomentum() const {
		return super_type::GetStackData().getMomentum(super_type::GetIndex());
	}
	corsika::Point getPosition() const {
		return super_type::GetStackData().getPosition(super_type::GetIndex());
	}
	corsika::units::si::TimeType getTime() const {
		return super_type::GetStackData().getTime(super_type::GetIndex());
	}
	/**
	 * @name derived quantities
	 *
	 * @{
	 */
	corsika::Vector<corsika::units::si::dimensionless_d> getDirection() const {
		return  this->getMomentum() /  this->getEnergy();
	}

	corsika::units::si::HEPMassType getMass() const {
		return corsika::GetMass(this->getPID());
	}

	int16_t getChargeNumber() const {
		return corsika::GetChargeNumber(this->getPID());
	}
	///@}
};

/**
 * Memory implementation of the most simple (stupid) particle stack object.
 *
 */

class SuperStupidStackImpl {

public:

	typedef  corsika::Vector<corsika::units::si::hepmomentum_d>        momentum_type;
	typedef  std::vector<corsika::Code>                             code_vector_type;
	typedef  std::vector<corsika::units::si::HEPEnergyType>       energy_vector_type;
	typedef  std::vector<corsika::Point>                           point_vector_type;
	typedef  std::vector<corsika::units::si::TimeType>              time_vector_type;
	typedef  std::vector<momentum_type>                         momentum_vector_type;

	SuperStupidStackImpl()=default;

	SuperStupidStackImpl( SuperStupidStackImpl const& other)=default;

	SuperStupidStackImpl( SuperStupidStackImpl && other)=default;


	SuperStupidStackImpl& operator=( SuperStupidStackImpl const& other)=default;

	SuperStupidStackImpl& operator=( SuperStupidStackImpl && other)=default;


	void init() {}
	void dump() const {}

	void clear() {
		dataPID_.clear();
		dataE_.clear();
		momentum_.clear();
		position_.clear();
		time_.clear();
	}

	unsigned int getSize() const { return dataPID_.size(); }
	unsigned int getCapacity() const { return dataPID_.size(); }

	void setPID(size_t i, const corsika::Code id) {
		dataPID_[i] = id;
	}
	void setEnergy(size_t i,  corsika::units::si::HEPEnergyType  const& e) {
		dataE_[i] = e;
	}
	void setMomentum(size_t i, momentum_type const& v) {
		momentum_[i] = v;
	}
	void setPosition(size_t i, corsika::Point const& v) {
		position_[i] = v;
	}
	void setTime(size_t i, corsika::units::si::TimeType const& v) {
		time_[i] = v;
	}

	corsika::Code getPID(size_t i) const {
		return dataPID_[i];
	}

	corsika::units::si::HEPEnergyType getEnergy(size_t i) const {
		return dataE_[i];
	}

	momentum_type getMomentum(size_t i) const {
		return momentum_[i];
	}

	corsika::Point getPosition(size_t i) const {
		return position_[i];
	}
	corsika::units::si::TimeType getTime(size_t i) const {
		return time_[i];
	}

	corsika::units::si::HEPEnergyType getDataE(size_t i) const {
		return dataE_[i];
	}

	void setDataE(size_t i, corsika::units::si::HEPEnergyType const& dataE) {
		dataE_[i] = dataE;
	}

	corsika::Code getDataPid(size_t i) const {
		return dataPID_;
	}

	void setDataPid(size_t i, corsika::Code  const& dataPid) {
		dataPID_[i] = dataPid;
	}
	/**
	 *   Function to copy particle at location i2 in stack to i1
	 */
	 void copy(size_t i1, size_t i2) {
		dataPID_[i2]  = dataPID_[i1];
		dataE_[i2]    = dataE_[i1];
		momentum_[i2] = momentum_[i1];
		position_[i2] = position_[i1];
		time_[i2]     = time_[i1];
	 }


	 /**
	  *   FIXME: change to iterators.
	  *   Function to copy particle at location i2 in stack to i1
	  */
	 void swap(size_t i1, size_t i2) {
		 std::swap(dataPID_[i2] , dataPID_[i1]);
		 std::swap(dataE_[i2]   , dataE_[i1]);
		 std::swap(momentum_[i2], momentum_[i1]);
		 std::swap(position_[i2], position_[i1]);
		 std::swap(time_[i2]    , time_[i1]);
	 }

	 void incrementSize() {
		 using corsika::Point;
		 using corsika::Code;

		 dataPID_.push_back(Code::Unknown);
		 dataE_.push_back(0 * corsika::units::si::electronvolt);

		 CoordinateSystem& dummyCS = RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

		 momentum_.push_back(momentum_type( dummyCS,
				 {0 * corsika::units::si::electronvolt, 0 * corsika::units::si::electronvolt,
						 0 * corsika::units::si::electronvolt}));

		 position_.push_back(
				 Point(dummyCS, {0 * corsika::units::si::meter, 0 * corsika::units::si::meter,
						 0 * corsika::units::si::meter}));
		 time_.push_back(0 * corsika::units::si::second);
	 }

	 void decrementSize() {
		 if (dataE_.size() > 0) {
			 dataPID_.pop_back();
			 dataE_.pop_back();
			 momentum_.pop_back();
			 position_.pop_back();
			 time_.pop_back();
		 }
	 }


private:

	 /// the actual memory to store particle data
	 code_vector_type dataPID_;
	 energy_vector_type dataE_;
	 momentum_vector_type momentum_;
	 point_vector_type position_;
	 time_vector_type time_;

}; // end class SuperStupidStackImpl


typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;


} // namespace corsika

