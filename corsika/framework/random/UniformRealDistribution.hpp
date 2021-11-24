/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <random>

namespace corsika {

  template <typename Quantity>
  class UniformRealDistribution {

    typedef typename Quantity::value_type real_type;
    typedef std::uniform_real_distribution<real_type> distribution_type;

  public:
    typedef Quantity value_type;

    UniformRealDistribution() = delete;

    UniformRealDistribution(Quantity const& b)
        : min_{value_type(phys::units::detail::magnitude_tag, 0)}
        , max_(b) {}

    UniformRealDistribution(value_type const& pmin, value_type const& pmax)
        : min_(pmin)
        , max_(pmax) {}

    UniformRealDistribution(UniformRealDistribution<value_type> const& other)
        : min_(other.getMin())
        , max_(other.getMax()) {}

    UniformRealDistribution<value_type>& operator=(
        UniformRealDistribution<value_type> const& other) {
      if (this == &other) return *this;
      min_ = other.getMin();
      max_ = other.getMax();
      return *this;
    }

    /**
     * Get the upper limit.
     *
     * @return value_type
     */
    value_type getMax() const { return max_; }

    /**
     * Set the upper limit.
     *
     * @param vMax
     */
    void setMax(value_type const& pmax) { max_ = pmax; }

    /**
     * Get the lower limit.
     *
     * @return The minimum possible value.
     */
    value_type getMin() const { return min_; }

    /**
     * Set the lower limit.
     *
     * @param pmin is the new lower bound.
     */
    void setMin(value_type const& pmin) { min_ = pmin; }

    /**
     * Generate a random number in the range [min, max].
     *
     * @tparam TGenerator
     * @param g
     * @return value_type
     */
    template <class TGenerator>
    value_type operator()(TGenerator& g) {
      return min_ + dist_(g) * (max_ - min_);
    }

  private:
    distribution_type dist_{real_type(0.), real_type(1.)};

    value_type min_;
    value_type max_;
  };

} // namespace corsika
