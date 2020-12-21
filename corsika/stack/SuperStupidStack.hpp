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
//#include <corsika/framework/geometry/RootCoordinateSystem.hpp> // remove
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace corsika::simple_stack {

  /**
   * Example of a particle object on the stack.
   */

  template <typename StackIteratorInterface>
  struct ParticleInterface : public ParticleBase<StackIteratorInterface> {

  private:
    typedef corsika::ParticleBase<StackIteratorInterface> super_type;

  public:
    std::string asString() const {
      using namespace corsika::units::si;
      return fmt::format("particle: i={}, PID={}, E={}GeV", super_type::getIndex(),
                         corsika::get_name(this->getPID()), this->getEnergy() / 1_GeV);
    }

    void setParticleData(std::tuple<corsika::Code, HEPEnergyType, MomentumVector,
                                    corsika::Point, TimeType> const& v);

    void setParticleData(ParticleInterface<StackIteratorInterface> const&,
                         std::tuple<corsika::Code, HEPEnergyType, MomentumVector,
                                    corsika::Point, TimeType> const& v);

    /// individual setters
    void setPID(corsika::Code const id) {
      super_type::getStackData().setPID(super_type::getIndex(), id);
    }
    void setEnergy(HEPEnergyType const& e) {
      super_type::getStackData().setEnergy(super_type::getIndex(), e);
    }
    void setMomentum(MomentumVector const& v) {
      super_type::getStackData().setMomentum(super_type::getIndex(), v);
    }
    void setPosition(corsika::Point const& v) {
      super_type::getStackData().setPosition(super_type::getIndex(), v);
    }
    void setTime(TimeType const& v) {
      super_type::getStackData().setTime(super_type::getIndex(), v);
    }

    /// individual getters
    corsika::Code getPID() const {
      return super_type::getStackData().getPID(super_type::getIndex());
    }
    HEPEnergyType getEnergy() const {
      return super_type::getStackData().getEnergy(super_type::getIndex());
    }
    MomentumVector getMomentum() const {
      return super_type::getStackData().getMomentum(super_type::getIndex());
    }
    corsika::Point getPosition() const {
      return super_type::getStackData().getPosition(super_type::getIndex());
    }
    TimeType getTime() const {
      return super_type::getStackData().getTime(super_type::getIndex());
    }
    /**
     * @name derived quantities
     *
     * @{
     */
    corsika::Vector<dimensionless_d> getDirection() const {
      return this->getMomentum() / this->getEnergy();
    }

    HEPMassType getMass() const { return corsika::get_mass(this->getPID()); }

    int16_t getChargeNumber() const { return corsika::get_charge_number(this->getPID()); }
    ///@}
  };

  /**
   * Memory implementation of the most simple (stupid) particle stack object.
   *
   */

  class SuperStupidStackImpl {

  public:
    typedef corsika::Vector<hepmomentum_d> momentum_type;
    typedef std::vector<corsika::Code> code_vector_type;
    typedef std::vector<HEPEnergyType> energy_vector_type;
    typedef std::vector<corsika::Point> point_vector_type;
    typedef std::vector<TimeType> time_vector_type;
    typedef std::vector<MomentumVector> momentum_vector_type;

    SuperStupidStackImpl() = default;

    SuperStupidStackImpl(SuperStupidStackImpl const& other) = default;

    SuperStupidStackImpl(SuperStupidStackImpl&& other) = default;

    SuperStupidStackImpl& operator=(SuperStupidStackImpl const& other) = default;

    SuperStupidStackImpl& operator=(SuperStupidStackImpl&& other) = default;

    void dump() const {}

    inline void clear();

    unsigned int getSize() const { return dataPID_.size(); }
    unsigned int getCapacity() const { return dataPID_.size(); }

    void setPID(size_t i, corsika::Code const id) { dataPID_[i] = id; }
    void setEnergy(size_t i, HEPEnergyType const& e) { dataE_[i] = e; }
    void setMomentum(size_t i, momentum_type const& v) { momentum_[i] = v; }
    void setPosition(size_t i, corsika::Point const& v) { position_[i] = v; }
    void setTime(size_t i, TimeType const& v) { time_[i] = v; }

    corsika::Code getPID(size_t i) const { return dataPID_[i]; }

    HEPEnergyType getEnergy(size_t i) const { return dataE_[i]; }

    momentum_type getMomentum(size_t i) const { return momentum_[i]; }

    corsika::Point getPosition(size_t i) const { return position_[i]; }
    TimeType getTime(size_t i) const { return time_[i]; }

    HEPEnergyType getDataE(size_t i) const { return dataE_[i]; }

    void setDataE(size_t i, HEPEnergyType const& dataE) { dataE_[i] = dataE; }

    corsika::Code getDataPid(size_t i) const { return dataPID_[i]; }

    void setDataPid(size_t i, corsika::Code const& dataPid) { dataPID_[i] = dataPid; }
    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    inline void copy(size_t i1, size_t i2);

    /**
     *   FIXME: change to iterators.
     *   Function to copy particle at location i2 in stack to i1
     */
    inline void swap(size_t i1, size_t i2);

    inline void incrementSize();

    inline void decrementSize();

  private:
    /// the actual memory to store particle data
    code_vector_type dataPID_;
    energy_vector_type dataE_;
    momentum_vector_type momentum_;
    point_vector_type position_;
    time_vector_type time_;

  }; // end class SuperStupidStackImpl

  typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

} // namespace corsika::simple_stack

#include <corsika/detail/stack/SuperStupidStack.inl>
