/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/logging/Logging.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <tuple>
#include <utility>
#include <vector>

namespace corsika::node {

  /**
   * Describe "volume node" data on a Stack.
   * 
   * Corresponding defintion of a stack-readout object, the iteractor
   * dereference operator will deliver access to these function
   * defintion of a stack-readout object, the iteractor dereference
   * operator will deliver access to these function
   */

  /**
   * \todo fixme: Document type T
   */
  template <typename T, typename TEnvType>
  struct GeometryDataInterface : public T {

    typedef T super_type;

  public:

    typedef typename TEnvType::BaseNodeType node_type;

    // default version for particle-creation from input data
    inline  void setParticleData(const std::tuple<node_type const*> v) {
      setNode(std::get<0>(v));
    }
    inline  void setParticleData(GeometryDataInterface& parent,
                         const std::tuple<node_type const*>) {
      setNode(parent.getNode()); // copy Node from parent particle!
    }
    inline  void setParticleData() { setNode(nullptr); }
    inline  void setParticleData(GeometryDataInterface& parent) {
      setNode(parent.getNode()); // copy Node from parent particle!
    }

    inline  std::string asString() const {
    	return fmt::format("node={}", fmt::ptr(getNode()));
    }

    inline  void setNode(node_type const* v) {

    	super_type::getStackData().setNode(super_type::getIndex(), v);
    }

    inline  node_type const* getNode() const {
    	return super_type::getStackData().getNode(super_type::getIndex());
    }
  };

  // definition of stack-data object to store geometry information


  /**
   * @class GeometryData
   *
   * definition of stack-data object to store geometry information
   */
  template <typename TEnvType>
  class GeometryData {

  public:

	typedef typename TEnvType::BaseNodeType node_type;
	typedef std::vector<const node_type*>   node_vector_type;

	GeometryData()= default;

	GeometryData( GeometryData<TEnvType> const& )= default;

	GeometryData( GeometryData<TEnvType> && )= default;

	GeometryData<TEnvType>&
	operator=( GeometryData<TEnvType> const& )= default;

	GeometryData<TEnvType>&
	operator=( GeometryData<TEnvType> && )= default;

    // these functions are needed for the Stack interface
	inline void clear() {
    	node_vector_.clear();
    }

	inline  unsigned int getSize() const {
    	return node_vector_.size();
    }

	inline  unsigned int getCapacity() const {
    	return node_vector_.size();
    }

	inline void copy(const int i1, const int i2) {
    	node_vector_[i2] = node_vector_[i1];
    }

	inline  void swap(const int i1, const int i2) {
    	std::swap(node_vector_[i1], node_vector_[i2]);
    }

    // custom data access function
	inline  void setNode(const int i, node_type const* v) {
    	node_vector_[i] = v;
    }

	inline  node_type const* getNode(const int i) const {
    	return node_vector_[i];
    }

    // these functions are also needed by the Stack interface
	inline  void incrementSize() {
    	node_vector_.push_back(nullptr);
    }

	inline  void decrementSize() {
      if (node_vector_.size() > 0) { node_vector_.pop_back(); }
    }

    // custom private data section
  private:

    node_vector_type node_vector_;
  };

  template <typename T, typename TEnv>
  struct MakeGeometryDataInterface {
    typedef GeometryDataInterface<T, TEnv> type;
  };

} // namespace corsika::stack::node
