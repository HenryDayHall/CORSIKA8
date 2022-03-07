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

#include <cnpy.hpp>
#include <boost/filesystem.hpp>
#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  /**
   * A common abstract interface for radio antennas.
   *
   * All concrete antenna implementations should be of
   * type Antenna<T> where T is a concrete antenna implementation.
   *
   */
  template <typename TAntennaImpl>
  class Antenna {

  public:
    std::string const name_;    ///< The name/identifier of this antenna.
    Point const location_;      ///< The location of this antenna.
    std::string filename_ = ""; ///< The filename for the output file for this antenna.

    // this stores the polarization vector of an electric field
    using ElectricFieldVector = Vector<ElectricFieldType::dimension_type>;
    using VectorPotential = Vector<VectorPotentialType::dimension_type>;

    /**
     * \brief Construct a base antenna instance.
     *
     * @param name    A name for this antenna.
     * @param location    The location of this antenna.
     *
     */
    Antenna(std::string const& name, Point const& location);

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
    Point const& getLocation() const;

    /**
     * Get the name of this name antenna.
     *
     * This is used in producing the output data file.
     */
    std::string const& getName() const;

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset();

    /**
     * Return a reference to the x-axis labels (i.e. time or frequency).
     *
     * This should be an xtensor-convertible type with
     * a ->data() method that converts to a raw pointer.
     */
    std::vector<long double> getAxis() const;

    /**
     * Return a reference to the underlying data for X polarization.
     *
     * This is used when writing the antenna information to disk
     * and will be converted to a 32-bit float before writing.
     */
    std::vector<double>& getDataX() const;

    /**
     * Return a reference to the underlying data for Y polarization.
     *
     * This is used when writing the antenna information to disk
     * and will be converted to a 32-bit float before writing.
     */
    std::vector<double>& getDataY() const;

    /**
     * Return a reference to the underlying data for Z polarization.
     *
     * This is used when writing the antenna information to disk
     * and will be converted to a 32-bit float before writing.
     */
    std::vector<double>& getDataZ() const;

    /**
     * Prepare for the start of the library.
     */
    void startOfLibrary(boost::filesystem::path const& directory,
                        std::string const radioImplementation);

    /**
     * Flush the data from this shower to disk.
     */
    void endOfShower(int const event, std::string const radioImplementation,
                     double const sampleRate);

  protected:
    /**
     * Get a reference to the underlying radio implementation.
     */
    TAntennaImpl& implementation();

  }; // END: class Antenna final

} // namespace corsika

#include <corsika/detail/modules/radio/antennas/Antenna.inl>