n/*
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
  class ExponentialDistribution {

    using RealType = typename TQuantity::value_type;
    typedef std::exponential_distribution<RealType> distribution_type;
    typedef  TQuantity quantity_type;

  public:

    ExponentialDistribution()=delete;

    ExponentialDistribution(quantity_type vBeta)
        : beta_(vBeta) {}

    ExponentialDistribution(ExponentialDistribution<quantity_type>const& other):
    beta_(other.getBeta())
    {}

    ExponentialDistribution<quantity_type>&
	operator=(ExponentialDistribution<quantity_type>const& other){
    	if( this == &other) return *this;
    	beta_ = other.getBeta();
    	return *this;
    }

    /**
     * @fn quantity_type getBeta()const
     * @brief Get parameter of exponential distribution \f[ \beta e^{-X}\f]
     * @pre
     * @post
     * @return  quantity_type
     */
    quantity_type getBeta() const {
		return beta_;
	}

    /**
     * @fn void setBeta(quantity_type)
     * @brief Set parameter of exponential distribution \f[ \beta e^{-X}\f]
     *
     * @pre
     * @post
     * @param vBeta
     */
	void setBeta(quantity_type vBeta) {
		beta_ = vBeta;
	}

	/**
     * @fn quantity_type operator ()(Generator&)
	 * @brief Generate a random number distributed like \f[ \beta e^{-X}\f]
	 *
	 * @pre
	 * @post
	 * @tparam Generator
	 * @param g
	 * @return
	 */
    template <class Generator>
    quantity_type operator()(Generator& g) {
      return beta_ * dist_(g);
    }

  private:

    distribution_type dist_{1.};
    quantity_type beta_;
  };

} // namespace corsika
