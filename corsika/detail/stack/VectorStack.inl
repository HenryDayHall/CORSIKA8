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
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace corsika {

  template <typename StackIteratorInterface>
  inline void ParticleInterface<StackIteratorInterface>::setParticleData(
      particle_data_momentum_type const& v) {
    this->setPID(std::get<0>(v));
    MomentumVector const p = std::get<1>(v);
    auto const P2 = p.getSquaredNorm();
    HEPMassType const M = getMass();
    this->setKineticEnergy(sqrt(P2 + square(M)) - M);
    if (P2 == static_pow<2>(0_eV)) {
      this->setDirection(DirectionVector(p.getCoordinateSystem(), {0, 0, 0}));
    } else {
      this->setDirection(p / sqrt(P2));
    }
    this->setPosition(std::get<2>(v));
    this->setTime(std::get<3>(v));
  }

  template <typename StackIteratorInterface>
  inline void ParticleInterface<StackIteratorInterface>::setParticleData(
      ParticleInterface<StackIteratorInterface> const&,
      particle_data_momentum_type const& v) {
    this->setPID(std::get<0>(v));
    MomentumVector const p = std::get<1>(v);
    auto const P2 = p.getSquaredNorm();
    HEPMassType const M = getMass();
    this->setKineticEnergy(sqrt(P2 + square(M)) - M);
    if (P2 == static_pow<2>(0_eV)) {
      this->setDirection(DirectionVector(p.getCoordinateSystem(), {0, 0, 0}));
    } else {
      this->setDirection(p / sqrt(P2));
    }
    this->setPosition(std::get<2>(v));
    this->setTime(std::get<3>(v));
  }

  template <typename StackIteratorInterface>
  inline void ParticleInterface<StackIteratorInterface>::setParticleData(
      particle_data_type const& v) {
    this->setPID(std::get<0>(v));
    this->setKineticEnergy(std::get<1>(v));
    this->setDirection(std::get<2>(v));
    this->setPosition(std::get<3>(v));
    this->setTime(std::get<4>(v));
  }

  template <typename StackIteratorInterface>
  inline void ParticleInterface<StackIteratorInterface>::setParticleData(
      ParticleInterface<StackIteratorInterface> const&, particle_data_type const& v) {
    this->setPID(std::get<0>(v));
    this->setKineticEnergy(std::get<1>(v));
    this->setDirection(std::get<2>(v));
    this->setPosition(std::get<3>(v));
    this->setTime(std::get<4>(v));
  }

  template <typename StackIteratorInterface>
  inline std::string ParticleInterface<StackIteratorInterface>::asString() const {
    return fmt::format("particle: i={}, PID={}, Ekin={}GeV", super_type::getIndex(),
                       get_name(this->getPID()), this->getKineticEnergy() / 1_GeV);
  }

  inline void VectorStackImpl::clear() {
    dataPID_.clear();
    dataEkin_.clear();
    direction_.clear();
    position_.clear();
    time_.clear();
  }

  inline void VectorStackImpl::copy(size_t i1, size_t i2) {
    dataPID_[i2] = dataPID_[i1];
    dataEkin_[i2] = dataEkin_[i1];
    direction_[i2] = direction_[i1];
    position_[i2] = position_[i1];
    time_[i2] = time_[i1];
  }

  inline void VectorStackImpl::swap(size_t i1, size_t i2) {
    std::swap(dataPID_[i2], dataPID_[i1]);
    std::swap(dataEkin_[i2], dataEkin_[i1]);
    std::swap(direction_[i2], direction_[i1]);
    std::swap(position_[i2], position_[i1]);
    std::swap(time_[i2], time_[i1]);
  }

  inline void VectorStackImpl::incrementSize() {
    dataPID_.push_back(Code::Unknown);
    dataEkin_.push_back(0 * electronvolt);

    CoordinateSystemPtr const& dummyCS = get_root_CoordinateSystem();

    direction_.push_back(DirectionVector(dummyCS, {0, 0, 0}));

    position_.push_back(Point(dummyCS, {0 * meter, 0 * meter, 0 * meter}));
    time_.push_back(0 * second);
  }

  inline void VectorStackImpl::decrementSize() {
    if (dataEkin_.size() > 0) {
      dataPID_.pop_back();
      dataEkin_.pop_back();
      direction_.pop_back();
      position_.pop_back();
      time_.pop_back();
    }
  }

} // namespace corsika
