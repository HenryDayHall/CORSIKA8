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

#include <corsika/modules/radio/antennas/Antenna.hpp>

namespace corsika {

    template <typename TAntennaImpl>
    inline Antenna<TAntennaImpl>::Antenna(std::string const& name, Point const& location)
            : name_(name)
            , location_(location){};

    template <typename TAntennaImpl>
    inline Point const& Antenna<TAntennaImpl>::getLocation() const { return location_; }

    template <typename TAntennaImpl>
    inline std::string const& Antenna<TAntennaImpl>::getName() const { return name_; }

    template <typename TAntennaImpl>
    inline void Antenna<TAntennaImpl>::startOfLibrary(const boost::filesystem::path &directory,
                                                      const std::string radioImplementation) {

        // calculate and save our filename
        filename_ = (directory / this->getName()).string() + ".npz";

        // get the axis labels for this antenna and write the first row.
        std::vector<long double> axis = this->implementation().getAxis();

        // check for the axis name
        std::string label = "Unknown";
        if constexpr (TAntennaImpl::is_time_domain) {
            label = "Time";
        } else if constexpr (TAntennaImpl::is_freq_domain) {
            label = "Frequency";
        }

        if (radioImplementation == "ZHS" && TAntennaImpl::is_time_domain) {
            std::cout << "ZHS" << std::endl;
            for (size_t i=0; i<axis.size()-1;i++)
            {
                axis.at(i) = (axis.at(i+1)+axis.at(i))/2.;
            }
            // explicitly convert the arrays to the needed type for cnpy
            axis.pop_back();
            long double const* raw_data = axis.data();
            std::vector<size_t> N = {axis.size()}; // cnpy needs a vector here -- this should be axis.size() - 1 --
            // write the labels to the first row of the NumPy file
            cnpy::npz_save(filename_, label, raw_data, N, "w");
        } else {
            // explicitly convert the arrays to the needed type for cnpy
            long double const* raw_data = axis.data();
            std::vector<size_t> N = {axis.size()}; // cnpy needs a vector here
            // write the labels to the first row of the NumPy file
            cnpy::npz_save(filename_, label, raw_data, N, "w");
        }
    }

    template <typename TAntennaImpl>
    inline void Antenna<TAntennaImpl>::endOfShower(const int event, const std::string radioImplementation,
                                                   const double sampleRate) {

        // get the copy of the waveform data for this event
        // we transpose it so that we can match dimensions with the
        // time array that is already in the output file
        std::vector<double> dataX = this->implementation().getDataX();
        std::vector<double> dataY = this->implementation().getDataY();
        std::vector<double> dataZ = this->implementation().getDataZ();

        if (radioImplementation == "ZHS") {
            std::vector<double> electricFieldX (dataX.size() - 1, 0); //num_bins_, std::vector<double>(3, 0)
            std::vector<double> electricFieldY (dataY.size() - 1, 0);
            std::vector<double> electricFieldZ (dataZ.size() - 1, 0);
            for (size_t i = 0; i < electricFieldX.size(); i++)
            {
                electricFieldX.at(i) = -(dataX.at(i+1)-dataX.at(i))*sampleRate;
                electricFieldY.at(i) = -(dataY.at(i+1)-dataY.at(i))*sampleRate;
                electricFieldZ.at(i) = -(dataZ.at(i+1)-dataZ.at(i))*sampleRate;
            }
            // cnpy needs a vector for the shape
            std::vector<size_t> shapeX = {electricFieldX.size()};
            std::vector<size_t> shapeY = {electricFieldY.size()};
            std::vector<size_t> shapeZ = {electricFieldZ.size()};
            cnpy::npz_save(filename_, std::to_string(event) + "X", electricFieldX.data(), shapeX, "a");
            cnpy::npz_save(filename_, std::to_string(event) + "Y", electricFieldY.data(), shapeY, "a");
            cnpy::npz_save(filename_, std::to_string(event) + "Z", electricFieldZ.data(), shapeZ, "a");
        } else {
            // cnpy needs a vector for the shape
            std::vector<size_t> shapeX = {dataX.size()};
            std::vector<size_t> shapeY = {dataY.size()};
            std::vector<size_t> shapeZ = {dataZ.size()};
            // and write this event to the .npz archive
            cnpy::npz_save(filename_, std::to_string(event) + "X", dataX.data(), shapeX, "a");
            cnpy::npz_save(filename_, std::to_string(event) + "Y", dataY.data(), shapeY, "a");
            cnpy::npz_save(filename_, std::to_string(event) + "Z", dataZ.data(), shapeZ, "a");
        }
    }

    template <typename TAntennaImpl>
    inline TAntennaImpl& Antenna<TAntennaImpl>::implementation() { return static_cast<TAntennaImpl&>(*this); }

} // namespace corsika