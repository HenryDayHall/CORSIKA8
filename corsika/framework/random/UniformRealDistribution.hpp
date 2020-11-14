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


  template <class TQuantity>
  class UniformRealDistribution {
    using RealType = typename TQuantity::value_type;
    typedef std::uniform_real_distribution<RealType>  distribution_type;

  public:

    typedef  TQuantity quantity_type;

    UniformRealDistribution() = delete;

    UniformRealDistribution(TQuantity b)
       : min_{quantity_type(phys::units::detail::magnitude_tag, 0)}
       , max_(b) {}

    UniformRealDistribution(quantity_type vMin, quantity_type vMax)
    : min_(vMin)
    , max_(vMax) {}

    UniformRealDistribution(UniformRealDistribution<quantity_type> const& other):
      min_(other.getMin())
    , max_(other.getMax())
    {}

    inline  UniformRealDistribution<quantity_type>&
	operator=(UniformRealDistribution<quantity_type> const& other){
    	if( this == &other) return *this;
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
    inline quantity_type getMax() const {
		return max_;
	}

    /**
     * @fn void setMax(quantity_type)
     * @brief Set the upper limit.
     *
     * @pre
     * @post
     * @param vMax
     */
    inline void setMax(quantity_type vMax) {
		max_ = vMax;
	}

    /**
     * @fn quantity_type getMin()const
     * @brief Get the lower limit.
     *
     * @pre
     * @post
     * @return
     */
    inline quantity_type getMin() const {
		return min_;
	}

    /**
     * @fn void setMin(quantity_type)
     * @brief  Set the lower limit.
     *
     * @pre
     * @post
     * @param vMin
     */
    inline void setMin(quantity_type vMin) {
		min_ = vMin;
	}

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
    inline quantity_type operator()(Generator& g) {
      return min_ + dist_(g) * (max_ - min_);
    }

  private:

    distribution_type dist_{RealType(0.), RealType(1.)};

    quantity_type min_;
    quantity_type max_;
  };

} // namespace corsika
