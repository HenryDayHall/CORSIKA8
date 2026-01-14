/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/interfaces/IRefractiveIndexModel.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <limits>

namespace corsika {
  namespace media {

    template <typename T, typename TGeometry>
    template <typename... Args>
    LinearTabulatedRefractiveIndex<T, TGeometry>::LinearTabulatedRefractiveIndex(
        std::filesystem::path const& tabulated_amosphere_path, TGeometry const& transformer,
        Args&&... args)
        : T(std::forward<Args>(args)...)
        , tabulated_amosphere_(tabulated_amosphere_path)
        , transformer_(transformer) {

      // Interpolate between the tabulated points in higher precision
    }

    template <typename T, typename TGeometry>
    std::vector<typename LinearTabulatedRefractiveIndex<T, TGeometry>::table_row_data> LinearTabulatedRefractiveIndex<T, TGeometry>::parse_file(
        std::string_view const& tabulated_amosphere_path) {
      // CSV read from file
      std::ifstream file(tabulated_amosphere_path);

      if (!file) { throw std::runtime_error("Failed to open file: " + std::string(tabulated_amosphere_path)); }

      std::string line;
      std::vector<table_row_data> refractive_index_data;

      for (int lineNumber = 0; std::getline(file, line); lineNumber++) {

        if (line.empty()) continue;
        if (line[0] == '#') continue;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        std::istringstream iss(line);
        table_row_data row;

        if (!(iss >> row.altitude >> row.density >> row.grammage >>
              row.refractive_index)) {
          throw std::runtime_error("Parse error at line " + std::to_string(lineNumber) +
                                   ": expected 4 numeric columns");
        }

        if (iss.fail()) {
          throw std::runtime_error("Could not parse value at line " +
                                   std::to_string(lineNumber));
        }

        if (!std::isfinite(row.altitude) || !std::isfinite(row.density) ||
            !std::isfinite(row.grammage) || !std::isfinite(row.refractive_index)) {
          throw std::runtime_error("Non-finite value at line " +
                                   std::to_string(lineNumber));
        }

        refractive_index_data.push_back(row);
      }

      if (refractive_index_data.size() < 2) {
        throw std::runtime_error("Less than two rows found in: " + tabulated_amosphere_.string());
      }

      std::sort(refractive_index_data.begin(), refractive_index_data.end(),
                [](table_row_data const& a, table_row_data const& b) {
                  return a.altitude < b.altitude;
                });

      return refractive_index_data;
    }

    template <typename T, typename TGeometry>
    void LinearTabulatedRefractiveIndex<T, TGeometry>::interpolate(
        std::vector<typename LinearTabulatedRefractiveIndex<T, TGeometry>::table_row_data> const& refractive_index_data) {

      if (refractive_index_data[0].altitude > refractive_index_data[1].altitude) {
        std::cerr << "Error in tabulated atmosphere file: heights are not sorted in "
                     "ascending order"
                  << std::endl;
        throw(std::runtime_error(
            "Error in tabulated atmosphere file: heights are not sorted "
            "in ascending order"));
      }

      for (int j = 1; j < refractive_index_data.size(); j++) {

        double data_height1 = refractive_index_data[j - 1].altitude;
        double data_height2 = refractive_index_data[j].altitude;

        double data_refractive_index1 =
            std::log(refractive_index_data[j - 1].refractive_index);
        double data_refractive_index2 =
            std::log(refractive_index_data[j].refractive_index);

        for (int interpol_height = 0; interpol_height < refractive_index_profile_.size();
             interpol_height++) {

          if (interpol_height >= data_height2) { break; }

          double height_diff = data_height2 - data_height1;
          double refractive_index_diff = data_refractive_index2 - data_refractive_index1;

          double fraction =
              (static_cast<double>(interpol_height) - data_height1) / height_diff;

          refractive_index_profile_[interpol_height] =
              std::exp(data_refractive_index1 + fraction * refractive_index_diff);
        }
      }
    }

    template <typename T, typename TGeometry>
    inline double LinearTabulatedRefractiveIndex<T, TGeometry>::getRefractiveIndex(
        Point const& point) const {
      double altitude = (transformer_.getEffectiveHeight(point) / 1_m);
      if (altitude < 0) {
        throw(std::runtime_error(
            "LinearTabulatedRefractiveIndex: point is inside the earth"));
      }
      if (altitude > 120'000) { return 1.0; }

      int lower = std::floor(altitude / 1.0);
      int upper = std::ceil(altitude / 1.0);
      double lower_index = refractive_index_profile_[lower];
      double upper_index = refractive_index_profile_[upper];
      double fraction = altitude - lower;
      return lower_index + fraction * (upper_index - lower_index);
    }

  } // namespace media
} // namespace corsika