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

  template <typename TStackIterator>
  class ParticleInterface : public ParticleBase<TStackIterator> {

  private:
    typedef ParticleBase<TStackIterator> super_type;

  public:
    typedef std::tuple<Code, HEPEnergyType, DirectionVector, Point, TimeType>
        particle_data_type;

    typedef std::tuple<Code, MomentumVector, Point, TimeType> particle_data_momentum_type;

    std::string asString() const;

    /**
     * Set data of new particle.
     *
     * @param v tuple containing: PID, Momentum Vector, Position, Time
     *
     *  MomentumVector is only used to determine the DirectionVector, the normalization
     * is lost.
     */
    void setParticleData(particle_data_type const& v);

    /**
     * Set data of new particle.
     *
     * @param p parent particle
     * @param v tuple containing: PID, Momentum Vector, Position, Time
     *
     *  MomentumVector is only used to determine the DirectionVector, the normalization
     * is lost.
     */
    void setParticleData(ParticleInterface<TStackIterator> const& p,
                         particle_data_type const& v);

    /**
     * Set data of new particle.
     *
     * @param v tuple containing: PID, kinetic Energy, Direction Vector, Position, Time
     *
     */
    void setParticleData(particle_data_momentum_type const& v);

    /**
     * Set data of new particle.
     *
     * @param p parent particle
     * @param v tuple containing: PID, kinetic Energy, Direction Vector, Position, Time
     *
     */
    void setParticleData(ParticleInterface<TStackIterator> const& p,
                         particle_data_momentum_type const& v);

    ///! Set particle corsika::Code
    void setPID(Code const id) {
      super_type::getStackData().setPID(super_type::getIndex(), id);
    }

    ///! Set energy
    void setEnergy(HEPEnergyType const& e) {
      super_type::getStackData().setKineticEnergy(super_type::getIndex(),
                                                  e - this->getMass());
    }

    ///! Set kinetic energy
    void setKineticEnergy(HEPEnergyType const& ekin) {
      super_type::getStackData().setKineticEnergy(super_type::getIndex(), ekin);
    }

    /**
       The MomentumVector v is used to determine the DirectionVector, and to update the
       particle energy.
    */
    void setMomentum(MomentumVector const& v) {
      HEPMomentumType const P = v.getNorm();
      if (P == 0_eV) {
        super_type::getStackData().setKineticEnergy(super_type::getIndex(), 0_eV);
        super_type::getStackData().setDirection(
            super_type::getIndex(), DirectionVector(v.getCoordinateSystem(), {0, 0, 0}));
      } else {
        super_type::getStackData().setKineticEnergy(
            super_type::getIndex(),
            sqrt(square(getMass()) + square(P)) - this->getMass());
        super_type::getStackData().setDirection(super_type::getIndex(), v / P);
      }
    }
    //! Set direction
    void setDirection(DirectionVector const& v) {
      super_type::getStackData().setDirection(super_type::getIndex(), v);
    }
    //! Set position
    void setPosition(Point const& v) {
      super_type::getStackData().setPosition(super_type::getIndex(), v);
    }
    //! Set time
    void setTime(TimeType const& v) {
      super_type::getStackData().setTime(super_type::getIndex(), v);
    }

    //! Get corsika::Code
    Code getPID() const {
      return super_type::getStackData().getPID(super_type::getIndex());
    }
    //! Get PDG code
    PDGCode getPDG() const { return get_PDG(getPID()); }
    //! Get kinetic energy
    HEPEnergyType getKineticEnergy() const {
      return super_type::getStackData().getKineticEnergy(super_type::getIndex());
    }
    //! Get direction
    DirectionVector getDirection() const {
      return super_type::getStackData().getDirection(super_type::getIndex());
    }
    //! Get position
    Point getPosition() const {
      return super_type::getStackData().getPosition(super_type::getIndex());
    }
    //! Get time
    TimeType getTime() const {
      return super_type::getStackData().getTime(super_type::getIndex());
    }
    /**
     * @name derived quantities
     *
     * @{
     */
    //! Get velocity
    VelocityVector getVelocity() const {
      return this->getMomentum() / this->getEnergy() * constants::c;
    }
    //! Get momentum
    MomentumVector getMomentum() const {
      auto const P = sqrt(square(getEnergy()) - square(this->getMass()));
      return super_type::getStackData().getDirection(super_type::getIndex()) * P;
    }
    //! Get mass of particle
    HEPMassType getMass() const { return get_mass(this->getPID()); }

    //! Get electric charge
    ElectricChargeType getCharge() const { return get_charge(this->getPID()); }

    //! Get kinetic energy
    HEPEnergyType getEnergy() const { return this->getKineticEnergy() + this->getMass(); }

    //! Get charge number
    int16_t getChargeNumber() const { return get_charge_number(this->getPID()); }
    ///@}
  };

  /**
   * Memory implementation of the most simple (stupid) particle stack object.
   *
   * @note if we ever want to have off-shell particles, we need to
   *       add momentum as HEPMomentumType, and a lot of care.
   */

  class VectorStackImpl {

  public:
    typedef std::vector<Code> code_vector_type;
    typedef std::vector<HEPEnergyType> kinetic_energy_vector_type;
    typedef std::vector<Point> point_vector_type;
    typedef std::vector<TimeType> time_vector_type;
    typedef std::vector<DirectionVector> direction_vector_type;

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
    void setKineticEnergy(size_t i, HEPEnergyType const& e) { dataEkin_[i] = e; }
    void setDirection(size_t i, DirectionVector const& v) { direction_[i] = v; }
    void setPosition(size_t i, Point const& v) { position_[i] = v; }
    void setTime(size_t i, TimeType const& v) { time_[i] = v; }

    Code getPID(size_t i) const { return dataPID_[i]; }
    HEPEnergyType getKineticEnergy(size_t i) const { return dataEkin_[i]; }
    DirectionVector getDirection(size_t i) const { return direction_[i]; }
    Point getPosition(size_t i) const { return position_[i]; }
    TimeType getTime(size_t i) const { return time_[i]; }

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
    kinetic_energy_vector_type dataEkin_;
    direction_vector_type direction_;
    point_vector_type position_;
    time_vector_type time_;

  }; // end class VectorStackImpl

  typedef Stack<VectorStackImpl, ParticleInterface> VectorStack;

} // namespace corsika

#include <corsika/detail/stack/VectorStack.inl>
