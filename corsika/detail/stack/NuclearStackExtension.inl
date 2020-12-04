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

#include <algorithm>
#include <tuple>
#include <vector>

namespace corsika::nuclear_stack {

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
void NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::setParticleData(particle_data_type const& v)
{

      if (std::get<0>(v) == Code::Nucleus) {
        std::ostringstream err;
        err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
        throw std::runtime_error(err.str());
      }

      super_type::setParticleData(v);
      setNucleusRef(-1); // this is not a nucleus
    }

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
void NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::setParticleData(altenative_particle_data_type const& v)
{
      const unsigned short A = std::get<5>(v);
      const unsigned short Z = std::get<6>(v);
      if (std::get<0>(v) != Code::Nucleus || A == 0 || Z == 0) {
        std::ostringstream err;
        err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
        throw std::runtime_error(err.str());
      }
      setNucleusRef(
          super_type::getStackData().getNucleusNextRef()); // store this nucleus data ref
      setNuclearA(A);
      setNuclearZ(Z);
      super_type::setParticleData(particle_data_type{std::get<0>(v), std::get<1>(v),
                                                     std::get<2>(v), std::get<3>(v),
                                                     std::get<4>(v)});
    }

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
void NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::setParticleData(super_type& p, particle_data_type const& v) {
    if (std::get<0>(v) == Code::Nucleus) {
      std::ostringstream err;
      err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
      throw std::runtime_error(err.str());
    }

    super_type::setParticleData(
        p, particle_data_type{std::get<0>(v), std::get<1>(v), std::get<2>(v),
                              std::get<3>(v), std::get<4>(v)});

    setNucleusRef(-1); // this is not a nucleus
  }

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
void NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::setParticleData(super_type& p, altenative_particle_data_type const& v) {

    const unsigned short A = std::get<5>(v);
    const unsigned short Z = std::get<6>(v);

    if (std::get<0>(v) != Code::Nucleus || A == 0 || Z == 0) {
      std::ostringstream err;
      err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
      throw std::runtime_error(err.str());
    }

    setNucleusRef(
        super_type::getStackData().getNucleusNextRef()); // store this nucleus data ref
    setNuclearA(A);
    setNuclearZ(Z);
    super_type::setParticleData(
        p, particle_data_type{std::get<0>(v), std::get<1>(v), std::get<2>(v),
                              std::get<3>(v), std::get<4>(v)});
  }

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
std::string NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::as_string() const {
    return fmt::format(
        "{}, nuc({})", super_type::as_string(),
        (isNucleus() ? fmt::format("A={}, Z={}", getNuclearA(), getNuclearZ())
                     : "n/a"));
  }


template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
HEPMassType NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::getMass() const {
    if (super_type::getPID() == Code::Nucleus)
      return getNucleusMass(getNuclearA(), getNuclearZ());
    return super_type::getMass();
  }

template <template <typename> class InnerParticleInterface,
          typename StackIteratorInterface>
int16_t NuclearParticleInterface< InnerParticleInterface,StackIteratorInterface>::getChargeNumber() const {
    if (super_type::getPID() == Code::Nucleus) return getNuclearZ();
    return super_type::getChargeNumber();
  }

template <typename InnerStackImpl>
int NuclearStackExtensionImpl<InnerStackImpl>::getNucleusNextRef(){
    nuclearA_.push_back(0);
    nuclearZ_.push_back(0);
    return nuclearA_.size() - 1;
  }

template <typename InnerStackImpl>
int NuclearStackExtensionImpl<InnerStackImpl>::getNucleusRef(const unsigned int i) const {
    if (nucleusRef_[i] >= 0) return nucleusRef_[i];
    std::ostringstream err;
    err << "NuclearStackExtension: no nucleus at ref=" << i;
    throw std::runtime_error(err.str());
  }

template <typename InnerStackImpl>
void NuclearStackExtensionImpl<InnerStackImpl>::copy(const unsigned int i1, const unsigned int i2) {
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

template <typename InnerStackImpl>
void NuclearStackExtensionImpl<InnerStackImpl>::clear() {
    super_type::clear();
    nucleusRef_.clear();
    nuclearA_.clear();
    nuclearZ_.clear();
  }

template <typename InnerStackImpl>
void NuclearStackExtensionImpl<InnerStackImpl>::swap(const unsigned int i1, const unsigned int i2) {
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


template <typename InnerStackImpl>
void NuclearStackExtensionImpl<InnerStackImpl>::incrementSize() {
    super_type::incrementSize();
    nucleusRef_.push_back(-1);
  }

template <typename InnerStackImpl>
void NuclearStackExtensionImpl<InnerStackImpl>::decrementSize() {
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



} // namespace corsika::nuclear_stack
