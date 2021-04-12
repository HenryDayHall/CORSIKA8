/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <tuple>
#include <utility>
#include <vector>

namespace corsika::weights {

  /**
   * Describe "particle weights" on a Stack.
   *
   * Corresponding defintion of a stack-readout object, the iteractor
   * dereference operator will deliver access to these function
   * defintion of a stack-readout object, the iteractor dereference
   * operator will deliver access to these function
   */

  /**
   * \tparam TParentStack The stack to be exteneded here with weight data
   */
  template <typename TParentStack>
  struct WeightDataInterface : public TParentStack {

    typedef TParentStack super_type;

  public:
    // default version for particle-creation from input data
    void setParticleData(std::tuple<double> const v) { setWeight(std::get<0>(v)); }
    void setParticleData(WeightDataInterface& parent, std::tuple<double> const) {
      setWeight(parent.getWeight()); // copy Weight from parent particle!
    }
    void setParticleData() { setWeight(1); } // default weight
    void setParticleData(WeightDataInterface& parent) {
      setWeight(parent.getWeight()); // copy Weight from parent particle!
    }

    std::string asString() const {
      return fmt::format("weight={}", fmt::ptr(getWeight()));
    }

    void setWeight(double const v) {

      super_type::getStackData().setWeight(super_type::getIndex(), v);
    }

    double getWeight() const {
      return super_type::getStackData().getWeight(super_type::getIndex());
    }
  };

  // definition of stack-data object to store geometry information

  /**
   * @class WeightData
   *
   * definition of stack-data object to store geometry information
   */
  class WeightData {

  public:
    typedef std::vector<double> weight_vector_type;

    WeightData() = default;

    WeightData(WeightData const&) = default;

    WeightData(WeightData&&) = default;

    WeightData& operator=(WeightData const&) = default;

    WeightData& operator=(WeightData&&) = default;

    // these functions are needed for the Stack interface
    void clear() { weight_vector_.clear(); }

    unsigned int getSize() const { return weight_vector_.size(); }

    unsigned int getCapacity() const { return weight_vector_.size(); }

    void copy(int const i1, int const i2) { weight_vector_[i2] = weight_vector_[i1]; }

    void swap(int const i1, int const i2) {
      std::swap(weight_vector_[i1], weight_vector_[i2]);
    }

    // custom data access function
    void setWeight(int const i, double const v) { weight_vector_[i] = v; }

    double getWeight(int const i) const { return weight_vector_[i]; }

    // these functions are also needed by the Stack interface
    void incrementSize() { weight_vector_.push_back(1); } // default weight

    void decrementSize() {
      if (weight_vector_.size() > 0) { weight_vector_.pop_back(); }
    }

    // custom private data section
  private:
    weight_vector_type weight_vector_;
  };

  template <typename TParentStack>
  struct MakeWeightDataInterface {
    typedef WeightDataInterface<TParentStack> type;
  };

} // namespace corsika::weights
