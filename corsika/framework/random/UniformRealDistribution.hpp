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
     * @fn quantity_type getMax()const
     * @brief Get the upper limit.
     *
     * @pre
     * @post
     * @return quantity_type
     */
    value_type getMax() const { return max_; }

    /**
     * @fn void setMax(quantity_type)
     * @brief Set the upper limit.
     *
     * @pre
     * @post
     * @param vMax
     */
    void setMax(value_type const& pmax) { max_ = pmax; }

    /**
     * @fn quantity_type getMin()const
     * @brief Get the lower limit.
     *
     * @pre
     * @post
     * @return
     */
    value_type getMin() const { return min_; }

    /**
     * @fn void setMin(quantity_type)
     * @brief  Set the lower limit.
     *
     * @pre
     * @post
     * @param vMin
     */
    void setMin(value_type const& pmin) { min_ = pmin; }

    /**
     * @fn quantity_type operator ()(Generator&)
     * @brief Generate a random numberin the range [min, max]
     *
     * @pre
     * @post
     * @tparam Generator
     * @param g
     * @return quantity_type
     */
    template <class Generator>
    value_type operator()(Generator& g) {
      return min_ + dist_(g) * (max_ - min_);
    }

  private:
    distribution_type dist_{real_type(0.), real_type(1.)};

    value_type min_;
    value_type max_;
  };

} // namespace corsika
