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
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace corsika {

  /**
   * Example of a particle object on the stack.
   */

  template <typename StackIteratorInterface>
  class ParticleInterface : public ParticleBase<StackIteratorInterface> {

  private:
    typedef ParticleBase<StackIteratorInterface> super_type;

  public:
    std::string asString() const {
      return fmt::format("particle: i={}, PID={}, E={}GeV", super_type::getIndex(),
                         get_name(this->getPID()), this->getEnergy() / 1_GeV);
    }

    void setParticleData(
        std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType> const& v);

    void setParticleData(
        ParticleInterface<StackIteratorInterface> const&,
        std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType> const& v);

    /// individual setters
    void setPID(Code const id) {
      super_type::getStackData().setPID(super_type::getIndex(), id);
    }
    void setEnergy(HEPEnergyType const& e) {
      super_type::getStackData().setEnergy(super_type::getIndex(), e);
    }
    void setMomentum(MomentumVector const& v) {
      super_type::getStackData().setMomentum(super_type::getIndex(), v);
    }
    void setPosition(Point const& v) {
      super_type::getStackData().setPosition(super_type::getIndex(), v);
    }
    void setTime(TimeType const& v) {
      super_type::getStackData().setTime(super_type::getIndex(), v);
    }

    /// individual getters
    Code getPID() const {
      return super_type::getStackData().getPID(super_type::getIndex());
    }
    HEPEnergyType getEnergy() const {
      return super_type::getStackData().getEnergy(super_type::getIndex());
    }
    MomentumVector getMomentum() const {
      return super_type::getStackData().getMomentum(super_type::getIndex());
    }
    Point getPosition() const {
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
    DirectionVector getDirection() const {
      return this->getMomentum() / this->getEnergy();
    }

    VelocityVector getVelocity() const {
      return this->getMomentum() / this->getEnergy() * constants::c;
    }

    HEPMassType getMass() const { return get_mass(this->getPID()); }

    ElectricChargeType getCharge() const { return get_charge(this->getPID()); }

    HEPEnergyType getKineticEnergy() const { return this->getEnergy() - this->getMass(); }

    int16_t getChargeNumber() const { return get_charge_number(this->getPID()); }
    ///@}
  };

  /**
   * Memory implementation of the most simple (stupid) particle stack object.
   *
   */

  class VectorStackImpl {

  public:
    typedef std::vector<Code> code_vector_type;
    typedef std::vector<HEPEnergyType> energy_vector_type;
    typedef std::vector<Point> point_vector_type;
    typedef std::vector<TimeType> time_vector_type;
    typedef std::vector<MomentumVector> momentum_vector_type;

    VectorStackImpl() = default;

    VectorStackImpl(VectorStackImpl const& other) = default;

    VectorStackImpl(VectorStackImpl&& other) = default;

    VectorStackImpl& operator=(VectorStackImpl const& other) = default;

    VectorStackImpl& operator=(VectorStackImpl&& other) = default;

    void dump() const {}

    void clear();

    unsigned int getSize() const { return dataPID_.size(); }
    unsigned int getCapacity() const { return dataPID_.size(); }

    void setPID(size_t i, Code const id) { dataPID_[i] = id; }
    void setEnergy(size_t i, HEPEnergyType const& e) { dataE_[i] = e; }
    void setMomentum(size_t i, MomentumVector const& v) { momentum_[i] = v; }
    void setPosition(size_t i, Point const& v) { position_[i] = v; }
    void setTime(size_t i, TimeType const& v) { time_[i] = v; }

    Code getPID(size_t i) const { return dataPID_[i]; }

    HEPEnergyType getEnergy(size_t i) const { return dataE_[i]; }

    MomentumVector getMomentum(size_t i) const { return momentum_[i]; }

    Point getPosition(size_t i) const { return position_[i]; }
    TimeType getTime(size_t i) const { return time_[i]; }

    HEPEnergyType getDataE(size_t i) const { return dataE_[i]; }

    void setDataE(size_t i, HEPEnergyType const& dataE) { dataE_[i] = dataE; }

    Code getDataPid(size_t i) const { return dataPID_[i]; }

    void setDataPid(size_t i, Code const& dataPid) { dataPID_[i] = dataPid; }
    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    void copy(size_t i1, size_t i2);

    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    void swap(size_t i1, size_t i2);

    void incrementSize();
    void decrementSize();

  private:
    /// the actual memory to store particle data
    code_vector_type dataPID_;
    energy_vector_type dataE_;
    momentum_vector_type momentum_;
    point_vector_type position_;
    time_vector_type time_;

  }; // end class VectorStackImpl

  typedef Stack<VectorStackImpl, ParticleInterface> VectorStack;

} // namespace corsika

#include <corsika/detail/stack/VectorStack.inl>
