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
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>
#include <boost/filesystem.hpp>
#include <corsika/framework/geometry/Point.hpp>

using namespace xt::placeholders;

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

  protected:
    /**
     * Get a reference to the underlying radio implementation.
     */
    TAntennaImpl& implementation() { return static_cast<TAntennaImpl&>(*this); }

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
    Antenna(std::string const& name, Point const& location)
        : name_(name)
        , location_(location){};

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

    /**
     * Return a reference to the x-axis labels (i.e. time or frequency).
     *
     * This should be an xtensor-convertible type with
     * a ->data() method that converts to a raw pointer.
     */
    xt::xtensor<double, 1> getAxis() const;

    /**
     * Return a reference to the underlying data.
     *
     * This is used when writing the antenna information to disk
     * and will be converted to a 32-bit float before writing.
     */
    xt::xtensor<double, 2>& getData() const;

    /**
     * Prepare for the start of the library.
     */
    void startOfLibrary(boost::filesystem::path const& directory, std::string const radioImplementation) {

      // calculate and save our filename
      filename_ = (directory / this->getName()).string() + ".npz";

      // get the axis labels for this antenna and write the first row.
      xt::xtensor<double, 1> axis = xt::cast<double>(this->implementation().getAxis());

      // check for the axis name
      std::string label = "Unknown";
      if constexpr (TAntennaImpl::is_time_domain) {
        label = "Time";
      } else if constexpr (TAntennaImpl::is_freq_domain) {
        label = "Frequency";
      }

      if (radioImplementation == "ZHS" && TAntennaImpl::is_time_domain) {
        for (size_t i=0; i<axis.size()-1;i++)
        {
            axis.at(i) = (axis.at(i+1)+axis.at(i))/2.;
        }
        // explicitly convert the arrays to the needed type for cnpy
        double const* raw_data = xt::view(axis,xt::range(_, -1)).data();
        std::vector<size_t> N = {axis.size()-1}; // cnpy needs a vector here
        // write the labels to the first row of the NumPy file
        cnpy::npz_save(filename_, label, raw_data, N, "w");
      } else {
        // explicitly convert the arrays to the needed type for cnpy
        double const* raw_data = axis.data();
        std::vector<size_t> N = {axis.size()}; // cnpy needs a vector here
        // write the labels to the first row of the NumPy file
        cnpy::npz_save(filename_, label, raw_data, N, "w");
      }
    }

    /**
     * Flush the data from this shower to disk.
     */
    void endOfShower(int const event, std::string const radioImplementation, double const sampleRate) {

      // get the copy of the waveform data for this event
      // we transpose it so that we can match dimensions with the
      // time array that is already in the output file
      xt::xtensor<double, 2> data = xt::transpose(xt::cast<double>(this->implementation().getData()));

      //std::cout << data << std::endl;
      if (radioImplementation == "ZHS") {
          xt::xtensor<double,2> electricField {xt::zeros<double>({data.shape()[0], data.shape()[1]-1})};
          for (size_t i = 0; i < electricField.shape()[1]; i++)
          {
              electricField.at(0, i) = -(data.at(0,i+1)-data.at(0,i))*sampleRate;
              electricField.at(1, i) = -(data.at(1,i+1)-data.at(1,i))*sampleRate;
              electricField.at(2, i) = -(data.at(2,i+1)-data.at(2,i))*sampleRate;
          }
          // cnpy needs a vector for the shape
          std::vector<size_t> shape = {electricField.shape()[0], electricField.shape()[1]};
          cnpy::npz_save(filename_, std::to_string(event), electricField.data(), shape, "a");
      } else {
          // cnpy needs a vector for the shape
          std::vector<size_t> shape = {data.shape()[0], data.shape()[1]};
          // and write this event to the .npz archive
          cnpy::npz_save(filename_, std::to_string(event), data.data(), shape, "a");
      }
    }



  }; // END: class Antenna final

} // namespace corsika
