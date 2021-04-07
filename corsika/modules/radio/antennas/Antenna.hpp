/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  /**
   * A common abstract interface for radio antennas.
   *
   * All concrete antenna implementations should be of
   * type Antenna<T> where T is a concrete antenna implementation.
   *
   */
  template <typename AntennaImpl>
  class Antenna {



  public:
    std::string const name_;         ///< The name/identifier of this antenna.
    Point const location_;           ///< The location of this antenna.

    // this stores the polarization vector of an electric field
    using ElectricFieldVector =
    QuantityVector<ElectricFieldType::dimension_type>;
//    using MagneticFieldVector =
//        QuantityVector<MagneticFieldType::dimension_type>;

    // a dimensionless vector used for the incident direction
//    using Vector = QuantityVector<dimensionless_d>;

    /**
     * \brief Construct a base antenna instance.
     *
     * @param name    A name for this antenna.
     * @param location    The location of this antenna.
     *
     */
    Antenna(std::string const& name, Point const& location)
        : name_(name)
        , location_(location){};

    // copy constructor
    Antenna(const Antenna& Ant)
        : name_(Ant.name_)
        , location_(Ant.location_){};

    /**
     * Receive a signal at this antenna.
     *
     * This is a general implementation call that must be specialized
     * for the particular antenna implementation and usage.
     *
     */
    template <typename... TVArgs>
    void receive(TVArgs&&... args);

    /**
     * Get the location of this antenna.
     */
    Point const& getLocation() const { return location_; };

    /**
     * Get the name of this name antenna.
     *
     * This is used in producing the output data file.
     */
    std::string const& getName() const { return name_; };

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset();

  }; // END: class Antenna final

} // namespace corsika
