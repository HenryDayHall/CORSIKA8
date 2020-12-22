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
  void ParticleInterface<StackIteratorInterface>::setParticleData(
      std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType> const& v) {
    this->setPID(std::get<0>(v));
    this->setEnergy(std::get<1>(v));
    this->setMomentum(std::get<2>(v));
    this->setPosition(std::get<3>(v));
    this->setTime(std::get<4>(v));
  }

  template <typename StackIteratorInterface>
  void ParticleInterface<StackIteratorInterface>::setParticleData(
      ParticleInterface<StackIteratorInterface> const&,
      std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType> const& v) {
    this->setPID(std::get<0>(v));
    this->setEnergy(std::get<1>(v));
    this->setMomentum(std::get<2>(v));
    this->setPosition(std::get<3>(v));
    this->setTime(std::get<4>(v));
  }

  inline void SuperStupidStackImpl::clear() {
    dataPID_.clear();
    dataE_.clear();
    momentum_.clear();
    position_.clear();
    time_.clear();
  }

  inline void SuperStupidStackImpl::copy(size_t i1, size_t i2) {
    dataPID_[i2] = dataPID_[i1];
    dataE_[i2] = dataE_[i1];
    momentum_[i2] = momentum_[i1];
    position_[i2] = position_[i1];
    time_[i2] = time_[i1];
  }

  inline void SuperStupidStackImpl::swap(size_t i1, size_t i2) {
    std::swap(dataPID_[i2], dataPID_[i1]);
    std::swap(dataE_[i2], dataE_[i1]);
    std::swap(momentum_[i2], momentum_[i1]);
    std::swap(position_[i2], position_[i1]);
    std::swap(time_[i2], time_[i1]);
  }

  inline void SuperStupidStackImpl::incrementSize() {
    dataPID_.push_back(Code::Unknown);
    dataE_.push_back(0 * electronvolt);

    CoordinateSystemPtr const& dummyCS = get_root_CoordinateSystem();

    momentum_.push_back(
        MomentumVector(dummyCS, {0 * electronvolt, 0 * electronvolt, 0 * electronvolt}));

    position_.push_back(Point(dummyCS, {0 * meter, 0 * meter, 0 * meter}));
    time_.push_back(0 * second);
  }

  inline void SuperStupidStackImpl::decrementSize() {
    if (dataE_.size() > 0) {
      dataPID_.pop_back();
      dataE_.pop_back();
      momentum_.pop_back();
      position_.pop_back();
      time_.pop_back();
    }
  }

} // namespace corsika
