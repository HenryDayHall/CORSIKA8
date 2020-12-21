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
#include <corsika/stack/SuperStupidStack.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

namespace corsika::nuclear_stack {

  /**
   *
   * Define ParticleInterface for NuclearStackExtension Stack derived from
   * ParticleInterface of Inner stack class
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
   * particle.getPID()==Code::Nucleus </code> before attempting to
   * read any nuclear information.
   */
  template <template <typename> class InnerParticleInterface,
            typename StackIteratorInterface>
  struct NuclearParticleInterface
      : public InnerParticleInterface<StackIteratorInterface> {

    typedef InnerParticleInterface<StackIteratorInterface> super_type;

  public:
    typedef std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType>
        particle_data_type;

    typedef std::tuple<Code, HEPEnergyType, MomentumVector, Point, TimeType,
                       unsigned short, unsigned short>
        altenative_particle_data_type;

    inline void setParticleData(particle_data_type const& v);

    inline void setParticleData(altenative_particle_data_type const& v);

    inline void setParticleData(super_type& p, particle_data_type const& v);

    inline void setParticleData(super_type& p, altenative_particle_data_type const& v);

    inline std::string asString() const;

    /**
     * @name individual setters
     * @{
     */
    inline void setNuclearA(const unsigned short vA) {
      super_type::getStackData().setNuclearA(super_type::getIndex(), vA);
    }
    inline void setNuclearZ(const unsigned short vZ) {
      super_type::getStackData().setNuclearZ(super_type::getIndex(), vZ);
    }
    /// @}

    /**
     * @name individual getters
     * @{
     */
    inline int getNuclearA() const {
      return super_type::getStackData().getNuclearA(super_type::getIndex());
    }
    inline int getNuclearZ() const {
      return super_type::getStackData().getNuclearZ(super_type::getIndex());
    }
    /// @}

    /**
     * Overwrite normal getParticleMass function with nuclear version
     */
    inline HEPMassType getMass() const;
    /**
     * Overwirte normal getChargeNumber function with nuclear version
     **/
    inline int16_t getChargeNumber() const;

    inline int getNucleusRef() const {
      return super_type::getStackData().getNucleusRef(super_type::getIndex());
    } // LCOV_EXCL_LINE

  protected:
    inline void setNucleusRef(const int vR) {
      super_type::getStackData().setNucleusRef(super_type::getIndex(), vR);
    }

    inline bool isNucleus() const {
      return super_type::getStackData().isNucleus(super_type::getIndex());
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
   * getNucleusA would produce an exception.
   */
  template <typename InnerStackImpl>
  class NuclearStackExtensionImpl : public InnerStackImpl {

    typedef InnerStackImpl super_type;

  public:
    typedef std::vector<int> nucleus_ref_type;
    typedef std::vector<unsigned short> nuclear_a_type;
    typedef std::vector<unsigned short> nuclear_z_type;

    NuclearStackExtensionImpl() = default;

    NuclearStackExtensionImpl(NuclearStackExtensionImpl<InnerStackImpl> const&) = default;

    NuclearStackExtensionImpl(NuclearStackExtensionImpl<InnerStackImpl>&&) = default;

    NuclearStackExtensionImpl<InnerStackImpl>& operator=(
        NuclearStackExtensionImpl<InnerStackImpl> const&) = default;

    NuclearStackExtensionImpl<InnerStackImpl>& operator=(
        NuclearStackExtensionImpl<InnerStackImpl>&&) = default;

    void init() { super_type::init(); }

    void dump() { super_type::dump(); }

    inline void clear();

    unsigned int getSize() const { return nucleusRef_.size(); }

    unsigned int getCapacity() const { return nucleusRef_.capacity(); }

    inline void setNuclearA(const unsigned int i, const unsigned short vA) {
      nuclearA_[getNucleusRef(i)] = vA;
    }

    inline void setNuclearZ(const unsigned int i, const unsigned short vZ) {
      nuclearZ_[getNucleusRef(i)] = vZ;
    }

    inline void setNucleusRef(const unsigned int i, const int v) { nucleusRef_[i] = v; }

    inline int getNuclearA(const unsigned int i) const {
      return nuclearA_[getNucleusRef(i)];
    }

    inline int getNuclearZ(const unsigned int i) const {
      return nuclearZ_[getNucleusRef(i)];
    }
    // this function will create new storage for Nuclear Properties, and return the
    // reference to it
    inline int getNucleusNextRef();

    inline int getNucleusRef(const unsigned int i) const;

    inline bool isNucleus(const unsigned int i) const { return nucleusRef_[i] >= 0; }

    /**
     *   Function to copy particle at location i1 in stack to i2
     */
    inline void copy(const unsigned int i1, const unsigned int i2);
    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    inline void swap(const unsigned int i1, const unsigned int i2);

    inline void incrementSize();

    inline void decrementSize();

  private:
    /// the actual memory to store particle data

    nucleus_ref_type nucleusRef_;
    nuclear_a_type nuclearA_;
    nuclear_z_type nuclearZ_;

  }; // end class NuclearStackExtensionImpl

  template <typename TInnerStack, template <typename> typename PI_>
  using NuclearStackExtension =
      Stack<NuclearStackExtensionImpl<typename TInnerStack::stack_implementation_type>,
            PI_>;

  //
  template <typename TStackIter>
  using ExtendedParticleInterfaceType =
      NuclearParticleInterface<simple_stack::SuperStupidStack::pi_type, TStackIter>;

  // the particle data stack with extra nuclear information:
  using ParticleDataStack = NuclearStackExtension<simple_stack::SuperStupidStack,
                                                  ExtendedParticleInterfaceType>;

} // namespace corsika::nuclear_stack

#include <corsika/detail/stack/NuclearStackExtension.inl>
